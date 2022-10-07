/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * Copyright (c) 2010-2011 Juniper Networks, Inc.
 * All rights reserved.
 *
 * Portions of this software were developed by Robert N. M. Watson under
 * contract to Juniper Networks, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$KAME: in6_pcb.c,v 1.31 2001/05/21 05:45:10 jinmei Exp $
 */

/*-
 * Copyright (c) 1982, 1986, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)in_pcb.c	8.2 (Berkeley) 1/4/94
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_inet.h"
#include "opt_inet6.h"
#include "opt_ipsec.h"
#include "opt_route.h"
#include "opt_rss.h"

#include <sys/hash.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/domain.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/sockio.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/priv.h>
#include <sys/proc.h>
#include <sys/jail.h>

#include <vm/uma.h>

#include <net/if.h>
#include <net/if_var.h>
#include <net/if_llatbl.h>
#include <net/if_types.h>
#include <net/route.h>
#include <net/route/nhop.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_systm.h>
#include <netinet/tcp_var.h>
#include <netinet/ip6.h>
#include <netinet/ip_var.h>

#include <netinet6/ip6_var.h>
#include <netinet6/nd6.h>
#include <netinet/in_pcb.h>
#include <netinet/in_pcb_var.h>
#include <netinet6/in6_pcb.h>
#include <netinet6/in6_fib.h>
#include <netinet6/scope6_var.h>

int
in6_pcbsetport(struct in6_addr *laddr, struct inpcb *inp, struct ucred *cred)
{
	struct socket *so = inp->inp_socket;
	u_int16_t lport = 0;
	int error, lookupflags = 0;
#ifdef INVARIANTS
	struct inpcbinfo *pcbinfo = inp->inp_pcbinfo;
#endif

	INP_WLOCK_ASSERT(inp);
	INP_HASH_WLOCK_ASSERT(pcbinfo);

	error = prison_local_ip6(cred, laddr,
	    ((inp->inp_flags & IN6P_IPV6_V6ONLY) != 0));
	if (error)
		return(error);

	/* XXX: this is redundant when called from in6_pcbbind */
	if ((so->so_options & (SO_REUSEADDR|SO_REUSEPORT|SO_REUSEPORT_LB)) == 0)
		lookupflags = INPLOOKUP_WILDCARD;

	inp->inp_flags |= INP_ANONPORT;

	error = in_pcb_lport(inp, NULL, &lport, cred, lookupflags);
	if (error != 0)
		return (error);

	inp->inp_lport = lport;
	if (in_pcbinshash(inp) != 0) {
		inp->in6p_laddr = in6addr_any;
		inp->inp_lport = 0;
		return (EAGAIN);
	}

	return (0);
}

int
in6_pcbbind(struct inpcb *inp, struct sockaddr *nam,
    struct ucred *cred)
{
	struct socket *so = inp->inp_socket;
	struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)NULL;
	struct inpcbinfo *pcbinfo = inp->inp_pcbinfo;
	u_short	lport = 0;
	int error, lookupflags = 0;
	int reuseport = (so->so_options & SO_REUSEPORT);

	/*
	 * XXX: Maybe we could let SO_REUSEPORT_LB set SO_REUSEPORT bit here
	 * so that we don't have to add to the (already messy) code below.
	 */
	int reuseport_lb = (so->so_options & SO_REUSEPORT_LB);

	INP_WLOCK_ASSERT(inp);
	INP_HASH_WLOCK_ASSERT(pcbinfo);

	if (inp->inp_lport || !IN6_IS_ADDR_UNSPECIFIED(&inp->in6p_laddr))
		return (EINVAL);
	if ((so->so_options & (SO_REUSEADDR|SO_REUSEPORT|SO_REUSEPORT_LB)) == 0)
		lookupflags = INPLOOKUP_WILDCARD;
	if (nam == NULL) {
		if ((error = prison_local_ip6(cred, &inp->in6p_laddr,
		    ((inp->inp_flags & IN6P_IPV6_V6ONLY) != 0))) != 0)
			return (error);
	} else {
		sin6 = (struct sockaddr_in6 *)nam;
		KASSERT(sin6->sin6_family == AF_INET6,
		    ("%s: invalid address family for %p", __func__, sin6));
		KASSERT(sin6->sin6_len == sizeof(*sin6),
		    ("%s: invalid address length for %p", __func__, sin6));

		if ((error = sa6_embedscope(sin6, V_ip6_use_defzone)) != 0)
			return(error);

		if ((error = prison_local_ip6(cred, &sin6->sin6_addr,
		    ((inp->inp_flags & IN6P_IPV6_V6ONLY) != 0))) != 0)
			return (error);

		lport = sin6->sin6_port;
		if (IN6_IS_ADDR_MULTICAST(&sin6->sin6_addr)) {
			/*
			 * Treat SO_REUSEADDR as SO_REUSEPORT for multicast;
			 * allow compepte duplication of binding if
			 * SO_REUSEPORT is set, or if SO_REUSEADDR is set
			 * and a multicast address is bound on both
			 * new and duplicated sockets.
			 */
			if ((so->so_options & (SO_REUSEADDR|SO_REUSEPORT)) != 0)
				reuseport = SO_REUSEADDR|SO_REUSEPORT;
			/*
			 * XXX: How to deal with SO_REUSEPORT_LB here?
			 * Treat same as SO_REUSEPORT for now.
			 */
			if ((so->so_options &
			    (SO_REUSEADDR|SO_REUSEPORT_LB)) != 0)
				reuseport_lb = SO_REUSEADDR|SO_REUSEPORT_LB;
		} else if (!IN6_IS_ADDR_UNSPECIFIED(&sin6->sin6_addr)) {
			struct epoch_tracker et;
			struct ifaddr *ifa;

			sin6->sin6_port = 0;		/* yech... */
			NET_EPOCH_ENTER(et);
			if ((ifa = ifa_ifwithaddr((struct sockaddr *)sin6)) ==
			    NULL &&
			    (inp->inp_flags & INP_BINDANY) == 0) {
				NET_EPOCH_EXIT(et);
				return (EADDRNOTAVAIL);
			}

			/*
			 * XXX: bind to an anycast address might accidentally
			 * cause sending a packet with anycast source address.
			 * We should allow to bind to a deprecated address, since
			 * the application dares to use it.
			 */
			if (ifa != NULL &&
			    ((struct in6_ifaddr *)ifa)->ia6_flags &
			    (IN6_IFF_ANYCAST|IN6_IFF_NOTREADY|IN6_IFF_DETACHED)) {
				NET_EPOCH_EXIT(et);
				return (EADDRNOTAVAIL);
			}
			NET_EPOCH_EXIT(et);
		}
		if (lport) {
			struct inpcb *t;

			/* GROSS */
			if (ntohs(lport) <= V_ipport_reservedhigh &&
			    ntohs(lport) >= V_ipport_reservedlow &&
			    priv_check_cred(cred, PRIV_NETINET_RESERVEDPORT))
				return (EACCES);
			if (!IN6_IS_ADDR_MULTICAST(&sin6->sin6_addr) &&
			    priv_check_cred(inp->inp_cred, PRIV_NETINET_REUSEPORT) != 0) {
				t = in6_pcblookup_local(pcbinfo,
				    &sin6->sin6_addr, lport,
				    INPLOOKUP_WILDCARD, cred);
				if (t &&
				    ((inp->inp_flags2 & INP_BINDMULTI) == 0) &&
				    (so->so_type != SOCK_STREAM ||
				     IN6_IS_ADDR_UNSPECIFIED(&t->in6p_faddr)) &&
				    (!IN6_IS_ADDR_UNSPECIFIED(&sin6->sin6_addr) ||
				     !IN6_IS_ADDR_UNSPECIFIED(&t->in6p_laddr) ||
				     (t->inp_flags2 & INP_REUSEPORT) ||
				     (t->inp_flags2 & INP_REUSEPORT_LB) == 0) &&
				    (inp->inp_cred->cr_uid !=
				     t->inp_cred->cr_uid))
					return (EADDRINUSE);

				/*
				 * If the socket is a BINDMULTI socket, then
				 * the credentials need to match and the
				 * original socket also has to have been bound
				 * with BINDMULTI.
				 */
				if (t && (! in_pcbbind_check_bindmulti(inp, t)))
					return (EADDRINUSE);

#ifdef INET
				if ((inp->inp_flags & IN6P_IPV6_V6ONLY) == 0 &&
				    IN6_IS_ADDR_UNSPECIFIED(&sin6->sin6_addr)) {
					struct sockaddr_in sin;

					in6_sin6_2_sin(&sin, sin6);
					t = in_pcblookup_local(pcbinfo,
					    sin.sin_addr, lport,
					    INPLOOKUP_WILDCARD, cred);
					if (t &&
					    ((inp->inp_flags2 & INP_BINDMULTI) == 0) &&
					    (so->so_type != SOCK_STREAM ||
					     ntohl(t->inp_faddr.s_addr) ==
					      INADDR_ANY) &&
					    (inp->inp_cred->cr_uid !=
					     t->inp_cred->cr_uid))
						return (EADDRINUSE);

					if (t && (! in_pcbbind_check_bindmulti(inp, t)))
						return (EADDRINUSE);
				}
#endif
			}
			t = in6_pcblookup_local(pcbinfo, &sin6->sin6_addr,
			    lport, lookupflags, cred);
			if (t && (reuseport & inp_so_options(t)) == 0 &&
			    (reuseport_lb & inp_so_options(t)) == 0) {
				return (EADDRINUSE);
			}
#ifdef INET
			if ((inp->inp_flags & IN6P_IPV6_V6ONLY) == 0 &&
			    IN6_IS_ADDR_UNSPECIFIED(&sin6->sin6_addr)) {
				struct sockaddr_in sin;

				in6_sin6_2_sin(&sin, sin6);
				t = in_pcblookup_local(pcbinfo, sin.sin_addr,
				   lport, lookupflags, cred);
				if (t &&
				    (reuseport & inp_so_options(t)) == 0 &&
				    (reuseport_lb & inp_so_options(t)) == 0 &&
				    (ntohl(t->inp_laddr.s_addr) != INADDR_ANY ||
				        (t->inp_vflag & INP_IPV6PROTO) != 0)) {
					return (EADDRINUSE);
				}
			}
#endif
		}
		inp->in6p_laddr = sin6->sin6_addr;
	}
	if (lport == 0) {
		if ((error = in6_pcbsetport(&inp->in6p_laddr, inp, cred)) != 0) {
			/* Undo an address bind that may have occurred. */
			inp->in6p_laddr = in6addr_any;
			return (error);
		}
	} else {
		inp->inp_lport = lport;
		if (in_pcbinshash(inp) != 0) {
			inp->in6p_laddr = in6addr_any;
			inp->inp_lport = 0;
			return (EAGAIN);
		}
	}
	return (0);
}

/*
 *   Transform old in6_pcbconnect() into an inner subroutine for new
 *   in6_pcbconnect(): Do some validity-checking on the remote
 *   address (in mbuf 'nam') and then determine local host address
 *   (i.e., which interface) to use to access that remote host.
 *
 *   This preserves definition of in6_pcbconnect(), while supporting a
 *   slightly different version for T/TCP.  (This is more than
 *   a bit of a kludge, but cleaning up the internal interfaces would
 *   have forced minor changes in every protocol).
 */
static int
in6_pcbladdr(struct inpcb *inp, struct sockaddr_in6 *sin6,
    struct in6_addr *plocal_addr6)
{
	int error = 0;
	int scope_ambiguous = 0;
	struct in6_addr in6a;
	struct epoch_tracker et;

	INP_WLOCK_ASSERT(inp);
	INP_HASH_WLOCK_ASSERT(inp->inp_pcbinfo);	/* XXXRW: why? */

	if (sin6->sin6_port == 0)
		return (EADDRNOTAVAIL);

	if (sin6->sin6_scope_id == 0 && !V_ip6_use_defzone)
		scope_ambiguous = 1;
	if ((error = sa6_embedscope(sin6, V_ip6_use_defzone)) != 0)
		return(error);

	if (!CK_STAILQ_EMPTY(&V_in6_ifaddrhead)) {
		/*
		 * If the destination address is UNSPECIFIED addr,
		 * use the loopback addr, e.g ::1.
		 */
		if (IN6_IS_ADDR_UNSPECIFIED(&sin6->sin6_addr))
			sin6->sin6_addr = in6addr_loopback;
	}
	if ((error = prison_remote_ip6(inp->inp_cred, &sin6->sin6_addr)) != 0)
		return (error);

	NET_EPOCH_ENTER(et);
	error = in6_selectsrc_socket(sin6, inp->in6p_outputopts,
	    inp, inp->inp_cred, scope_ambiguous, &in6a, NULL);
	NET_EPOCH_EXIT(et);
	if (error)
		return (error);

	/*
	 * Do not update this earlier, in case we return with an error.
	 *
	 * XXX: this in6_selectsrc_socket result might replace the bound local
	 * address with the address specified by setsockopt(IPV6_PKTINFO).
	 * Is it the intended behavior?
	 */
	*plocal_addr6 = in6a;

	/*
	 * Don't do pcblookup call here; return interface in
	 * plocal_addr6
	 * and exit to caller, that will do the lookup.
	 */

	return (0);
}

/*
 * Outer subroutine:
 * Connect from a socket to a specified address.
 * Both address and port must be specified in argument sin.
 * If don't have a local address for this socket yet,
 * then pick one.
 */
int
in6_pcbconnect_mbuf(struct inpcb *inp, struct sockaddr *nam,
    struct ucred *cred, struct mbuf *m, bool rehash)
{
	struct inpcbinfo *pcbinfo = inp->inp_pcbinfo;
	struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)nam;
	struct sockaddr_in6 laddr6;
	int error;

	KASSERT(sin6->sin6_family == AF_INET6,
	    ("%s: invalid address family for %p", __func__, sin6));
	KASSERT(sin6->sin6_len == sizeof(*sin6),
	    ("%s: invalid address length for %p", __func__, sin6));

	bzero(&laddr6, sizeof(laddr6));
	laddr6.sin6_family = AF_INET6;

	INP_WLOCK_ASSERT(inp);
	INP_HASH_WLOCK_ASSERT(pcbinfo);

#ifdef ROUTE_MPATH
	if (CALC_FLOWID_OUTBOUND) {
		uint32_t hash_type, hash_val;

		hash_val = fib6_calc_software_hash(&inp->in6p_laddr,
		    &sin6->sin6_addr, 0, sin6->sin6_port,
		    inp->inp_socket->so_proto->pr_protocol, &hash_type);
		inp->inp_flowid = hash_val;
		inp->inp_flowtype = hash_type;
	}
#endif
	/*
	 * Call inner routine, to assign local interface address.
	 * in6_pcbladdr() may automatically fill in sin6_scope_id.
	 */
	if ((error = in6_pcbladdr(inp, sin6, &laddr6.sin6_addr)) != 0)
		return (error);

	if (in6_pcblookup_hash_locked(pcbinfo, &sin6->sin6_addr,
			       sin6->sin6_port,
			      IN6_IS_ADDR_UNSPECIFIED(&inp->in6p_laddr)
			      ? &laddr6.sin6_addr : &inp->in6p_laddr,
			      inp->inp_lport, 0, NULL, M_NODOM) != NULL) {
		return (EADDRINUSE);
	}
	if (IN6_IS_ADDR_UNSPECIFIED(&inp->in6p_laddr)) {
		if (inp->inp_lport == 0) {
			/*
			 * rehash was required to be true in the past for
			 * this case; retain that convention.  However,
			 * we now call in_pcb_lport_dest rather than
			 * in6_pcbbind; the former does not insert into
			 * the hash table, the latter does.  Change rehash
			 * to false to do the in_pcbinshash below.
			 */
			KASSERT(rehash == true,
			    ("Rehashing required for unbound inps"));
			rehash = false;
			error = in_pcb_lport_dest(inp,
			    (struct sockaddr *) &laddr6, &inp->inp_lport,
			    (struct sockaddr *) sin6, sin6->sin6_port, cred,
			    INPLOOKUP_WILDCARD);
			if (error)
				return (error);
		}
		inp->in6p_laddr = laddr6.sin6_addr;
	}
	inp->in6p_faddr = sin6->sin6_addr;
	inp->inp_fport = sin6->sin6_port;
	/* update flowinfo - draft-itojun-ipv6-flowlabel-api-00 */
	inp->inp_flow &= ~IPV6_FLOWLABEL_MASK;
	if (inp->inp_flags & IN6P_AUTOFLOWLABEL)
		inp->inp_flow |=
		    (htonl(ip6_randomflowlabel()) & IPV6_FLOWLABEL_MASK);

	if (rehash) {
		in_pcbrehash(inp);
	} else {
		in_pcbinshash(inp);
	}

	return (0);
}

int
in6_pcbconnect(struct inpcb *inp, struct sockaddr *nam, struct ucred *cred)
{

	return (in6_pcbconnect_mbuf(inp, nam, cred, NULL, true));
}

void
in6_pcbdisconnect(struct inpcb *inp)
{

	INP_WLOCK_ASSERT(inp);
	INP_HASH_WLOCK_ASSERT(inp->inp_pcbinfo);

	bzero((caddr_t)&inp->in6p_faddr, sizeof(inp->in6p_faddr));
	inp->inp_fport = 0;
	/* clear flowinfo - draft-itojun-ipv6-flowlabel-api-00 */
	inp->inp_flow &= ~IPV6_FLOWLABEL_MASK;
	in_pcbrehash(inp);
}

struct sockaddr *
in6_sockaddr(in_port_t port, struct in6_addr *addr_p)
{
	struct sockaddr_in6 *sin6;

	sin6 = malloc(sizeof *sin6, M_SONAME, M_WAITOK);
	bzero(sin6, sizeof *sin6);
	sin6->sin6_family = AF_INET6;
	sin6->sin6_len = sizeof(*sin6);
	sin6->sin6_port = port;
	sin6->sin6_addr = *addr_p;
	(void)sa6_recoverscope(sin6); /* XXX: should catch errors */

	return (struct sockaddr *)sin6;
}

struct sockaddr *
in6_v4mapsin6_sockaddr(in_port_t port, struct in_addr *addr_p)
{
	struct sockaddr_in sin;
	struct sockaddr_in6 *sin6_p;

	bzero(&sin, sizeof sin);
	sin.sin_family = AF_INET;
	sin.sin_len = sizeof(sin);
	sin.sin_port = port;
	sin.sin_addr = *addr_p;

	sin6_p = malloc(sizeof *sin6_p, M_SONAME,
		M_WAITOK);
	in6_sin_2_v4mapsin6(&sin, sin6_p);

	return (struct sockaddr *)sin6_p;
}

int
in6_getsockaddr(struct socket *so, struct sockaddr **nam)
{
	struct inpcb *inp;
	struct in6_addr addr;
	in_port_t port;

	inp = sotoinpcb(so);
	KASSERT(inp != NULL, ("in6_getsockaddr: inp == NULL"));

	INP_RLOCK(inp);
	port = inp->inp_lport;
	addr = inp->in6p_laddr;
	INP_RUNLOCK(inp);

	*nam = in6_sockaddr(port, &addr);
	return 0;
}

int
in6_getpeeraddr(struct socket *so, struct sockaddr **nam)
{
	struct inpcb *inp;
	struct in6_addr addr;
	in_port_t port;

	inp = sotoinpcb(so);
	KASSERT(inp != NULL, ("in6_getpeeraddr: inp == NULL"));

	INP_RLOCK(inp);
	port = inp->inp_fport;
	addr = inp->in6p_faddr;
	INP_RUNLOCK(inp);

	*nam = in6_sockaddr(port, &addr);
	return 0;
}

int
in6_mapped_sockaddr(struct socket *so, struct sockaddr **nam)
{
	struct	inpcb *inp;
	int	error;

	inp = sotoinpcb(so);
	KASSERT(inp != NULL, ("in6_mapped_sockaddr: inp == NULL"));

#ifdef INET
	if ((inp->inp_vflag & (INP_IPV4 | INP_IPV6)) == INP_IPV4) {
		error = in_getsockaddr(so, nam);
		if (error == 0)
			in6_sin_2_v4mapsin6_in_sock(nam);
	} else
#endif
	{
		/* scope issues will be handled in in6_getsockaddr(). */
		error = in6_getsockaddr(so, nam);
	}

	return error;
}

int
in6_mapped_peeraddr(struct socket *so, struct sockaddr **nam)
{
	struct	inpcb *inp;
	int	error;

	inp = sotoinpcb(so);
	KASSERT(inp != NULL, ("in6_mapped_peeraddr: inp == NULL"));

#ifdef INET
	if ((inp->inp_vflag & (INP_IPV4 | INP_IPV6)) == INP_IPV4) {
		error = in_getpeeraddr(so, nam);
		if (error == 0)
			in6_sin_2_v4mapsin6_in_sock(nam);
	} else
#endif
	/* scope issues will be handled in in6_getpeeraddr(). */
	error = in6_getpeeraddr(so, nam);

	return error;
}

/*
 * Pass some notification to all connections of a protocol
 * associated with address dst.  The local address and/or port numbers
 * may be specified to limit the search.  The "usual action" will be
 * taken, depending on the ctlinput cmd.  The caller must filter any
 * cmds that are uninteresting (e.g., no error in the map).
 * Call the protocol specific routine (if any) to report
 * any errors for each matching socket.
 */
static bool
inp_match6(const struct inpcb *inp, void *v __unused)
{

	return ((inp->inp_vflag & INP_IPV6) != 0);
}

void
in6_pcbnotify(struct inpcbinfo *pcbinfo, struct sockaddr_in6 *sa6_dst,
    u_int fport_arg, const struct sockaddr_in6 *src, u_int lport_arg,
    int errno, void *cmdarg,
    struct inpcb *(*notify)(struct inpcb *, int))
{
	struct inpcb_iterator inpi = INP_ITERATOR(pcbinfo, INPLOOKUP_WLOCKPCB,
	    inp_match6, NULL);
	struct inpcb *inp;
	struct sockaddr_in6 sa6_src;
	u_short	fport = fport_arg, lport = lport_arg;
	u_int32_t flowinfo;

	if (IN6_IS_ADDR_UNSPECIFIED(&sa6_dst->sin6_addr))
		return;

	/*
	 * note that src can be NULL when we get notify by local fragmentation.
	 */
	sa6_src = (src == NULL) ? sa6_any : *src;
	flowinfo = sa6_src.sin6_flowinfo;

	while ((inp = inp_next(&inpi)) != NULL) {
		INP_WLOCK_ASSERT(inp);
		/*
		 * If the error designates a new path MTU for a destination
		 * and the application (associated with this socket) wanted to
		 * know the value, notify.
		 * XXX: should we avoid to notify the value to TCP sockets?
		 */
		if (errno == EMSGSIZE && cmdarg != NULL)
			ip6_notify_pmtu(inp, sa6_dst, *(uint32_t *)cmdarg);

		/*
		 * Detect if we should notify the error. If no source and
		 * destination ports are specified, but non-zero flowinfo and
		 * local address match, notify the error. This is the case
		 * when the error is delivered with an encrypted buffer
		 * by ESP. Otherwise, just compare addresses and ports
		 * as usual.
		 */
		if (lport == 0 && fport == 0 && flowinfo &&
		    inp->inp_socket != NULL &&
		    flowinfo == (inp->inp_flow & IPV6_FLOWLABEL_MASK) &&
		    IN6_ARE_ADDR_EQUAL(&inp->in6p_laddr, &sa6_src.sin6_addr))
			goto do_notify;
		else if (!IN6_ARE_ADDR_EQUAL(&inp->in6p_faddr,
					     &sa6_dst->sin6_addr) ||
			 inp->inp_socket == 0 ||
			 (lport && inp->inp_lport != lport) ||
			 (!IN6_IS_ADDR_UNSPECIFIED(&sa6_src.sin6_addr) &&
			  !IN6_ARE_ADDR_EQUAL(&inp->in6p_laddr,
					      &sa6_src.sin6_addr)) ||
			 (fport && inp->inp_fport != fport)) {
			continue;
		}

	  do_notify:
		if (notify)
			(*notify)(inp, errno);
	}
}

/*
 * Lookup a PCB based on the local address and port.  Caller must hold the
 * hash lock.  No inpcb locks or references are acquired.
 */
struct inpcb *
in6_pcblookup_local(struct inpcbinfo *pcbinfo, struct in6_addr *laddr,
    u_short lport, int lookupflags, struct ucred *cred)
{
	struct inpcb *inp;
	int matchwild = 3, wildcard;

	KASSERT((lookupflags & ~(INPLOOKUP_WILDCARD)) == 0,
	    ("%s: invalid lookup flags %d", __func__, lookupflags));

	INP_HASH_LOCK_ASSERT(pcbinfo);

	if ((lookupflags & INPLOOKUP_WILDCARD) == 0) {
		struct inpcbhead *head;
		/*
		 * Look for an unconnected (wildcard foreign addr) PCB that
		 * matches the local address and port we're looking for.
		 */
		head = &pcbinfo->ipi_hashbase[INP_PCBHASH_WILD(lport,
		    pcbinfo->ipi_hashmask)];
		CK_LIST_FOREACH(inp, head, inp_hash) {
			/* XXX inp locking */
			if ((inp->inp_vflag & INP_IPV6) == 0)
				continue;
			if (IN6_IS_ADDR_UNSPECIFIED(&inp->in6p_faddr) &&
			    IN6_ARE_ADDR_EQUAL(&inp->in6p_laddr, laddr) &&
			    inp->inp_lport == lport) {
				/* Found. */
				if (cred == NULL ||
				    prison_equal_ip6(cred->cr_prison,
					inp->inp_cred->cr_prison))
					return (inp);
			}
		}
		/*
		 * Not found.
		 */
		return (NULL);
	} else {
		struct inpcbporthead *porthash;
		struct inpcbport *phd;
		struct inpcb *match = NULL;
		/*
		 * Best fit PCB lookup.
		 *
		 * First see if this local port is in use by looking on the
		 * port hash list.
		 */
		porthash = &pcbinfo->ipi_porthashbase[INP_PCBPORTHASH(lport,
		    pcbinfo->ipi_porthashmask)];
		CK_LIST_FOREACH(phd, porthash, phd_hash) {
			if (phd->phd_port == lport)
				break;
		}
		if (phd != NULL) {
			/*
			 * Port is in use by one or more PCBs. Look for best
			 * fit.
			 */
			CK_LIST_FOREACH(inp, &phd->phd_pcblist, inp_portlist) {
				wildcard = 0;
				if (cred != NULL &&
				    !prison_equal_ip6(cred->cr_prison,
					inp->inp_cred->cr_prison))
					continue;
				/* XXX inp locking */
				if ((inp->inp_vflag & INP_IPV6) == 0)
					continue;
				if (!IN6_IS_ADDR_UNSPECIFIED(&inp->in6p_faddr))
					wildcard++;
				if (!IN6_IS_ADDR_UNSPECIFIED(
					&inp->in6p_laddr)) {
					if (IN6_IS_ADDR_UNSPECIFIED(laddr))
						wildcard++;
					else if (!IN6_ARE_ADDR_EQUAL(
					    &inp->in6p_laddr, laddr))
						continue;
				} else {
					if (!IN6_IS_ADDR_UNSPECIFIED(laddr))
						wildcard++;
				}
				if (wildcard < matchwild) {
					match = inp;
					matchwild = wildcard;
					if (matchwild == 0)
						break;
				}
			}
		}
		return (match);
	}
}

static bool
in6_multi_match(const struct inpcb *inp, void *v __unused)
{

	if ((inp->inp_vflag & INP_IPV6) && inp->in6p_moptions != NULL)
		return (true);
	else
		return (false);
}

void
in6_pcbpurgeif0(struct inpcbinfo *pcbinfo, struct ifnet *ifp)
{
	struct inpcb_iterator inpi = INP_ITERATOR(pcbinfo, INPLOOKUP_RLOCKPCB,
	    in6_multi_match, NULL);
	struct inpcb *inp;
	struct in6_multi *inm;
	struct in6_mfilter *imf;
	struct ip6_moptions *im6o;

	IN6_MULTI_LOCK_ASSERT();

	while ((inp = inp_next(&inpi)) != NULL) {
		INP_RLOCK_ASSERT(inp);

		im6o = inp->in6p_moptions;
		/*
		 * Unselect the outgoing ifp for multicast if it
		 * is being detached.
		 */
		if (im6o->im6o_multicast_ifp == ifp)
			im6o->im6o_multicast_ifp = NULL;
		/*
		 * Drop multicast group membership if we joined
		 * through the interface being detached.
		 */
restart:
		IP6_MFILTER_FOREACH(imf, &im6o->im6o_head) {
			if ((inm = imf->im6f_in6m) == NULL)
				continue;
			if (inm->in6m_ifp != ifp)
				continue;
			ip6_mfilter_remove(&im6o->im6o_head, imf);
			in6_leavegroup_locked(inm, NULL);
			ip6_mfilter_free(imf);
			goto restart;
		}
	}
}

/*
 * Check for alternatives when higher level complains
 * about service problems.  For now, invalidate cached
 * routing information.  If the route was created dynamically
 * (by a redirect), time to try a default gateway again.
 */
void
in6_losing(struct inpcb *inp)
{

	RO_INVALIDATE_CACHE(&inp->inp_route6);
}

/*
 * After a routing change, flush old routing
 * and allocate a (hopefully) better one.
 */
struct inpcb *
in6_rtchange(struct inpcb *inp, int errno __unused)
{

	RO_INVALIDATE_CACHE(&inp->inp_route6);
	return inp;
}

static struct inpcb *
in6_pcblookup_lbgroup(const struct inpcbinfo *pcbinfo,
    const struct in6_addr *laddr, uint16_t lport, const struct in6_addr *faddr,
    uint16_t fport, int lookupflags, uint8_t numa_domain)
{
	struct inpcb *local_wild, *numa_wild;
	const struct inpcblbgrouphead *hdr;
	struct inpcblbgroup *grp;
	uint32_t idx;

	INP_HASH_LOCK_ASSERT(pcbinfo);

	hdr = &pcbinfo->ipi_lbgrouphashbase[
	    INP_PCBPORTHASH(lport, pcbinfo->ipi_lbgrouphashmask)];

	/*
	 * Order of socket selection:
	 * 1. non-wild.
	 * 2. wild (if lookupflags contains INPLOOKUP_WILDCARD).
	 *
	 * NOTE:
	 * - Load balanced group does not contain jailed sockets.
	 * - Load balanced does not contain IPv4 mapped INET6 wild sockets.
	 */
	local_wild = NULL;
	numa_wild = NULL;
	CK_LIST_FOREACH(grp, hdr, il_list) {
#ifdef INET
		if (!(grp->il_vflag & INP_IPV6))
			continue;
#endif
		if (grp->il_lport != lport)
			continue;

		idx = INP6_PCBLBGROUP_PKTHASH(faddr, lport, fport) %
		    grp->il_inpcnt;
		if (IN6_ARE_ADDR_EQUAL(&grp->il6_laddr, laddr)) {
			if (numa_domain == M_NODOM ||
			    grp->il_numa_domain == numa_domain) {
				return (grp->il_inp[idx]);
			}
			else
				numa_wild = grp->il_inp[idx];
		}
		if (IN6_IS_ADDR_UNSPECIFIED(&grp->il6_laddr) &&
		    (lookupflags & INPLOOKUP_WILDCARD) != 0 &&
		    (local_wild == NULL || numa_domain == M_NODOM ||
			grp->il_numa_domain == numa_domain)) {
			local_wild = grp->il_inp[idx];
		}
	}
	if (numa_wild != NULL)
		return (numa_wild);
	return (local_wild);
}

/*
 * Lookup PCB in hash list.  Used in in_pcb.c as well as here.
 */
struct inpcb *
in6_pcblookup_hash_locked(struct inpcbinfo *pcbinfo, struct in6_addr *faddr,
    u_int fport_arg, struct in6_addr *laddr, u_int lport_arg,
    int lookupflags, struct ifnet *ifp, uint8_t numa_domain)
{
	struct inpcbhead *head;
	struct inpcb *inp, *tmpinp;
	u_short fport = fport_arg, lport = lport_arg;

	KASSERT((lookupflags & ~(INPLOOKUP_WILDCARD)) == 0,
	    ("%s: invalid lookup flags %d", __func__, lookupflags));

	INP_HASH_LOCK_ASSERT(pcbinfo);

	/*
	 * First look for an exact match.
	 */
	tmpinp = NULL;
	head = &pcbinfo->ipi_hashbase[INP6_PCBHASH(faddr, lport, fport,
	    pcbinfo->ipi_hashmask)];
	CK_LIST_FOREACH(inp, head, inp_hash) {
		/* XXX inp locking */
		if ((inp->inp_vflag & INP_IPV6) == 0)
			continue;
		if (IN6_ARE_ADDR_EQUAL(&inp->in6p_faddr, faddr) &&
		    IN6_ARE_ADDR_EQUAL(&inp->in6p_laddr, laddr) &&
		    inp->inp_fport == fport &&
		    inp->inp_lport == lport) {
			/*
			 * XXX We should be able to directly return
			 * the inp here, without any checks.
			 * Well unless both bound with SO_REUSEPORT?
			 */
			if (prison_flag(inp->inp_cred, PR_IP6))
				return (inp);
			if (tmpinp == NULL)
				tmpinp = inp;
		}
	}
	if (tmpinp != NULL)
		return (tmpinp);

	/*
	 * Then look in lb group (for wildcard match).
	 */
	if ((lookupflags & INPLOOKUP_WILDCARD) != 0) {
		inp = in6_pcblookup_lbgroup(pcbinfo, laddr, lport, faddr,
		    fport, lookupflags, numa_domain);
		if (inp != NULL)
			return (inp);
	}

	/*
	 * Then look for a wildcard match, if requested.
	 */
	if ((lookupflags & INPLOOKUP_WILDCARD) != 0) {
		struct inpcb *local_wild = NULL, *local_exact = NULL;
		struct inpcb *jail_wild = NULL;
		int injail;

		/*
		 * Order of socket selection - we always prefer jails.
		 *      1. jailed, non-wild.
		 *      2. jailed, wild.
		 *      3. non-jailed, non-wild.
		 *      4. non-jailed, wild.
		 */
		head = &pcbinfo->ipi_hashbase[INP_PCBHASH_WILD(lport,
		    pcbinfo->ipi_hashmask)];
		CK_LIST_FOREACH(inp, head, inp_hash) {
			/* XXX inp locking */
			if ((inp->inp_vflag & INP_IPV6) == 0)
				continue;

			if (!IN6_IS_ADDR_UNSPECIFIED(&inp->in6p_faddr) ||
			    inp->inp_lport != lport) {
				continue;
			}

			injail = prison_flag(inp->inp_cred, PR_IP6);
			if (injail) {
				if (prison_check_ip6_locked(
				    inp->inp_cred->cr_prison, laddr) != 0)
					continue;
			} else {
				if (local_exact != NULL)
					continue;
			}

			if (IN6_ARE_ADDR_EQUAL(&inp->in6p_laddr, laddr)) {
				if (injail)
					return (inp);
				else
					local_exact = inp;
			} else if (IN6_IS_ADDR_UNSPECIFIED(&inp->in6p_laddr)) {
				if (injail)
					jail_wild = inp;
				else
					local_wild = inp;
			}
		} /* LIST_FOREACH */

		if (jail_wild != NULL)
			return (jail_wild);
		if (local_exact != NULL)
			return (local_exact);
		if (local_wild != NULL)
			return (local_wild);
	} /* if ((lookupflags & INPLOOKUP_WILDCARD) != 0) */

	/*
	 * Not found.
	 */
	return (NULL);
}

/*
 * Lookup PCB in hash list, using pcbinfo tables.  This variation locks the
 * hash list lock, and will return the inpcb locked (i.e., requires
 * INPLOOKUP_LOCKPCB).
 */
static struct inpcb *
in6_pcblookup_hash(struct inpcbinfo *pcbinfo, struct in6_addr *faddr,
    u_int fport, struct in6_addr *laddr, u_int lport, int lookupflags,
    struct ifnet *ifp, uint8_t numa_domain)
{
	struct inpcb *inp;

	smr_enter(pcbinfo->ipi_smr);
	inp = in6_pcblookup_hash_locked(pcbinfo, faddr, fport, laddr, lport,
	    lookupflags & INPLOOKUP_WILDCARD, ifp, numa_domain);
	if (inp != NULL) {
		if (__predict_false(inp_smr_lock(inp,
		    (lookupflags & INPLOOKUP_LOCKMASK)) == false))
			inp = NULL;
	} else
		smr_exit(pcbinfo->ipi_smr);

	return (inp);
}

/*
 * Public inpcb lookup routines, accepting a 4-tuple, and optionally, an mbuf
 * from which a pre-calculated hash value may be extracted.
 */
struct inpcb *
in6_pcblookup(struct inpcbinfo *pcbinfo, struct in6_addr *faddr, u_int fport,
    struct in6_addr *laddr, u_int lport, int lookupflags, struct ifnet *ifp)
{

	KASSERT((lookupflags & ~INPLOOKUP_MASK) == 0,
	    ("%s: invalid lookup flags %d", __func__, lookupflags));
	KASSERT((lookupflags & (INPLOOKUP_RLOCKPCB | INPLOOKUP_WLOCKPCB)) != 0,
	    ("%s: LOCKPCB not set", __func__));

	return (in6_pcblookup_hash(pcbinfo, faddr, fport, laddr, lport,
	    lookupflags, ifp, M_NODOM));
}

struct inpcb *
in6_pcblookup_mbuf(struct inpcbinfo *pcbinfo, struct in6_addr *faddr,
    u_int fport, struct in6_addr *laddr, u_int lport, int lookupflags,
    struct ifnet *ifp, struct mbuf *m)
{

	KASSERT((lookupflags & ~INPLOOKUP_MASK) == 0,
	    ("%s: invalid lookup flags %d", __func__, lookupflags));
	KASSERT((lookupflags & (INPLOOKUP_RLOCKPCB | INPLOOKUP_WLOCKPCB)) != 0,
	    ("%s: LOCKPCB not set", __func__));

	return (in6_pcblookup_hash(pcbinfo, faddr, fport, laddr, lport,
	    lookupflags, ifp, m->m_pkthdr.numa_domain));
}

void
init_sin6(struct sockaddr_in6 *sin6, struct mbuf *m, int srcordst)
{
	struct ip6_hdr *ip;

	ip = mtod(m, struct ip6_hdr *);
	bzero(sin6, sizeof(*sin6));
	sin6->sin6_len = sizeof(*sin6);
	sin6->sin6_family = AF_INET6;
	sin6->sin6_addr = srcordst ? ip->ip6_dst : ip->ip6_src;

	(void)sa6_recoverscope(sin6); /* XXX: should catch errors... */

	return;
}
