/*
 * System call numbers.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * $FreeBSD$
 * created from FreeBSD: src/sys/ia64/ia32/syscalls.master,v 1.21 2003/04/25 15:59:18 jhb Exp 
 */

#define	IA32_SYS_syscall	0
#define	IA32_SYS_exit	1
#define	IA32_SYS_fork	2
#define	IA32_SYS_read	3
#define	IA32_SYS_write	4
#define	IA32_SYS_ia32_open	5
#define	IA32_SYS_close	6
#define	IA32_SYS_ia32_wait4	7
				/* 8 is obsolete old creat */
#define	IA32_SYS_link	9
#define	IA32_SYS_unlink	10
				/* 11 is obsolete execv */
#define	IA32_SYS_chdir	12
#define	IA32_SYS_fchdir	13
#define	IA32_SYS_mknod	14
#define	IA32_SYS_chmod	15
#define	IA32_SYS_chown	16
#define	IA32_SYS_break	17
#define	IA32_SYS_ia32_getfsstat	18
				/* 19 is obsolete olseek */
#define	IA32_SYS_getpid	20
#define	IA32_SYS_mount	21
#define	IA32_SYS_unmount	22
#define	IA32_SYS_setuid	23
#define	IA32_SYS_getuid	24
#define	IA32_SYS_geteuid	25
#define	IA32_SYS_ptrace	26
#define	IA32_SYS_sendmsg	28
#define	IA32_SYS_recvfrom	29
#define	IA32_SYS_accept	30
#define	IA32_SYS_getpeername	31
#define	IA32_SYS_getsockname	32
#define	IA32_SYS_ia32_access	33
#define	IA32_SYS_ia32_chflags	34
#define	IA32_SYS_fchflags	35
#define	IA32_SYS_sync	36
#define	IA32_SYS_kill	37
#define	IA32_SYS_getppid	39
#define	IA32_SYS_dup	41
#define	IA32_SYS_pipe	42
#define	IA32_SYS_getegid	43
#define	IA32_SYS_profil	44
#define	IA32_SYS_ktrace	45
#define	IA32_SYS_getgid	47
#define	IA32_SYS_getlogin	49
#define	IA32_SYS_setlogin	50
#define	IA32_SYS_acct	51
#define	IA32_SYS_sigpending	52
#define	IA32_SYS_ia32_sigaltstack	53
#define	IA32_SYS_ioctl	54
#define	IA32_SYS_reboot	55
#define	IA32_SYS_revoke	56
#define	IA32_SYS_symlink	57
#define	IA32_SYS_readlink	58
#define	IA32_SYS_ia32_execve	59
#define	IA32_SYS_umask	60
#define	IA32_SYS_chroot	61
				/* 62 is obsolete ofstat */
				/* 63 is obsolete ogetkerninfo */
				/* 64 is obsolete ogetpagesize */
				/* 65 is obsolete omsync */
				/* 66 is obsolete ovfork */
				/* 67 is obsolete vread */
				/* 68 is obsolete vwrite */
#define	IA32_SYS_sbrk	69
#define	IA32_SYS_sstk	70
				/* 71 is obsolete ommap */
#define	IA32_SYS_vadvise	72
#define	IA32_SYS_munmap	73
#define	IA32_SYS_mprotect	74
#define	IA32_SYS_madvise	75
				/* 76 is obsolete vhangup */
				/* 77 is obsolete vlimit */
#define	IA32_SYS_mincore	78
#define	IA32_SYS_getgroups	79
#define	IA32_SYS_setgroups	80
#define	IA32_SYS_getpgrp	81
#define	IA32_SYS_setpgid	82
#define	IA32_SYS_ia32_setitimer	83
				/* 84 is obsolete owait */
				/* 85 is obsolete oswapon */
				/* 86 is obsolete ogetitimer */
				/* 87 is obsolete ogethostname */
				/* 88 is obsolete osethostname */
#define	IA32_SYS_getdtablesize	89
#define	IA32_SYS_dup2	90
#define	IA32_SYS_fcntl	92
#define	IA32_SYS_ia32_select	93
#define	IA32_SYS_fsync	95
#define	IA32_SYS_setpriority	96
#define	IA32_SYS_socket	97
#define	IA32_SYS_connect	98
#define	IA32_SYS_accept	99
#define	IA32_SYS_getpriority	100
				/* 101 is obsolete osend */
				/* 102 is obsolete orecv */
				/* 103 is obsolete osigreturn */
#define	IA32_SYS_bind	104
#define	IA32_SYS_setsockopt	105
#define	IA32_SYS_listen	106
				/* 107 is obsolete vtimes */
				/* 108 is obsolete osigvec */
				/* 109 is obsolete osigblock */
				/* 110 is obsolete osigsetmask */
				/* 111 is obsolete osigsuspend */
				/* 112 is obsolete osigstack */
				/* 113 is obsolete orecvmsg */
				/* 114 is obsolete osendmsg */
				/* 115 is obsolete vtrace */
#define	IA32_SYS_ia32_gettimeofday	116
#define	IA32_SYS_ia32_getrusage	117
#define	IA32_SYS_getsockopt	118
#define	IA32_SYS_ia32_readv	120
#define	IA32_SYS_ia32_writev	121
#define	IA32_SYS_ia32_settimeofday	122
#define	IA32_SYS_fchown	123
#define	IA32_SYS_fchmod	124
#define	IA32_SYS_recvfrom	125
#define	IA32_SYS_setreuid	126
#define	IA32_SYS_setregid	127
#define	IA32_SYS_rename	128
				/* 129 is obsolete otruncate */
				/* 130 is obsolete ftruncate */
#define	IA32_SYS_flock	131
#define	IA32_SYS_mkfifo	132
#define	IA32_SYS_sendto	133
#define	IA32_SYS_shutdown	134
#define	IA32_SYS_socketpair	135
#define	IA32_SYS_mkdir	136
#define	IA32_SYS_rmdir	137
#define	IA32_SYS_ia32_utimes	138
				/* 139 is obsolete 4.2 sigreturn */
#define	IA32_SYS_ia32_adjtime	140
				/* 141 is obsolete ogetpeername */
				/* 142 is obsolete ogethostid */
				/* 143 is obsolete sethostid */
				/* 144 is obsolete getrlimit */
				/* 145 is obsolete setrlimit */
				/* 146 is obsolete killpg */
#define	IA32_SYS_setsid	147
#define	IA32_SYS_quotactl	148
				/* 149 is obsolete oquota */
				/* 150 is obsolete ogetsockname */
#define	IA32_SYS_getdirentries	156
#define	IA32_SYS_ia32_statfs	157
#define	IA32_SYS_ia32_fstatfs	158
#define	IA32_SYS_getfh	161
#define	IA32_SYS_getdomainname	162
#define	IA32_SYS_setdomainname	163
#define	IA32_SYS_uname	164
#define	IA32_SYS_sysarch	165
#define	IA32_SYS_rtprio	166
#define	IA32_SYS_ia32_semsys	169
#define	IA32_SYS_ia32_msgsys	170
#define	IA32_SYS_ia32_shmsys	171
#define	IA32_SYS_ia32_pread	173
#define	IA32_SYS_ia32_pwrite	174
#define	IA32_SYS_ntp_adjtime	176
#define	IA32_SYS_setgid	181
#define	IA32_SYS_setegid	182
#define	IA32_SYS_seteuid	183
#define	IA32_SYS_ia32_stat	188
#define	IA32_SYS_ia32_fstat	189
#define	IA32_SYS_ia32_lstat	190
#define	IA32_SYS_pathconf	191
#define	IA32_SYS_fpathconf	192
#define	IA32_SYS_getrlimit	194
#define	IA32_SYS_setrlimit	195
#define	IA32_SYS_getdirentries	196
#define	IA32_SYS_ia32_mmap	197
#define	IA32_SYS___syscall	198
#define	IA32_SYS_ia32_lseek	199
#define	IA32_SYS_ia32_truncate	200
#define	IA32_SYS_ia32_ftruncate	201
#define	IA32_SYS_ia32_sysctl	202
#define	IA32_SYS_mlock	203
#define	IA32_SYS_munlock	204
#define	IA32_SYS_undelete	205
#define	IA32_SYS_futimes	206
#define	IA32_SYS_getpgid	207
#define	IA32_SYS_poll	209
#define	IA32_SYS___semctl	220
#define	IA32_SYS_semget	221
#define	IA32_SYS_semop	222
#define	IA32_SYS_msgctl	224
#define	IA32_SYS_msgget	225
#define	IA32_SYS_msgsnd	226
#define	IA32_SYS_msgrcv	227
#define	IA32_SYS_shmat	228
#define	IA32_SYS_shmctl	229
#define	IA32_SYS_shmdt	230
#define	IA32_SYS_shmget	231
#define	IA32_SYS_clock_gettime	232
#define	IA32_SYS_clock_settime	233
#define	IA32_SYS_clock_getres	234
#define	IA32_SYS_nanosleep	240
#define	IA32_SYS_minherit	250
#define	IA32_SYS_rfork	251
#define	IA32_SYS_openbsd_poll	252
#define	IA32_SYS_issetugid	253
#define	IA32_SYS_lchown	254
#define	IA32_SYS_getdents	272
#define	IA32_SYS_lchmod	274
#define	IA32_SYS_netbsd_lchown	275
#define	IA32_SYS_lutimes	276
#define	IA32_SYS_netbsd_msync	277
#define	IA32_SYS_nstat	278
#define	IA32_SYS_nfstat	279
#define	IA32_SYS_nlstat	280
#define	IA32_SYS_fhstatfs	297
#define	IA32_SYS_fhopen	298
#define	IA32_SYS_fhstat	299
#define	IA32_SYS_modnext	300
#define	IA32_SYS_modstat	301
#define	IA32_SYS_modfnext	302
#define	IA32_SYS_modfind	303
#define	IA32_SYS_kldload	304
#define	IA32_SYS_kldunload	305
#define	IA32_SYS_kldfind	306
#define	IA32_SYS_kldnext	307
#define	IA32_SYS_kldstat	308
#define	IA32_SYS_kldfirstmod	309
#define	IA32_SYS_getsid	310
#define	IA32_SYS_setresuid	311
#define	IA32_SYS_setresgid	312
				/* 313 is obsolete signanosleep */
#define	IA32_SYS_yield	321
				/* 322 is obsolete thr_sleep */
				/* 323 is obsolete thr_wakeup */
#define	IA32_SYS_mlockall	324
#define	IA32_SYS_munlockall	325
#define	IA32_SYS___getcwd	326
#define	IA32_SYS_sched_setparam	327
#define	IA32_SYS_sched_getparam	328
#define	IA32_SYS_sched_setscheduler	329
#define	IA32_SYS_sched_getscheduler	330
#define	IA32_SYS_sched_yield	331
#define	IA32_SYS_sched_get_priority_max	332
#define	IA32_SYS_sched_get_priority_min	333
#define	IA32_SYS_sched_rr_get_interval	334
#define	IA32_SYS_utrace	335
				/* 336 is old ia32_sendfile */
#define	IA32_SYS_kldsym	337
#define	IA32_SYS_jail	338
#define	IA32_SYS_sigprocmask	340
#define	IA32_SYS_sigsuspend	341
#define	IA32_SYS_ia32_sigaction	342
#define	IA32_SYS_sigpending	343
#define	IA32_SYS_sigreturn	344
#define	IA32_SYS___acl_get_file	347
#define	IA32_SYS___acl_set_file	348
#define	IA32_SYS___acl_get_fd	349
#define	IA32_SYS___acl_set_fd	350
#define	IA32_SYS___acl_delete_file	351
#define	IA32_SYS___acl_delete_fd	352
#define	IA32_SYS___acl_aclcheck_file	353
#define	IA32_SYS___acl_aclcheck_fd	354
#define	IA32_SYS_extattrctl	355
#define	IA32_SYS_extattr_set_file	356
#define	IA32_SYS_extattr_get_file	357
#define	IA32_SYS_extattr_delete_file	358
#define	IA32_SYS_getresuid	360
#define	IA32_SYS_getresgid	361
#define	IA32_SYS_kqueue	362
#define	IA32_SYS_kevent	363
#define	IA32_SYS_extattr_set_fd	371
#define	IA32_SYS_extattr_get_fd	372
#define	IA32_SYS_extattr_delete_fd	373
#define	IA32_SYS___setugid	374
#define	IA32_SYS_eaccess	376
#define	IA32_SYS_nmount	378
#define	IA32_SYS_kse_exit	379
#define	IA32_SYS_kse_wakeup	380
#define	IA32_SYS_kse_create	381
#define	IA32_SYS_kse_thr_interrupt	382
#define	IA32_SYS_kse_release	383
#define	IA32_SYS_kenv	390
#define	IA32_SYS_lchflags	391
#define	IA32_SYS_uuidgen	392
#define	IA32_SYS_ia32_sendfile	393
#define	IA32_SYS_thr_create	430
#define	IA32_SYS_thr_exit	431
#define	IA32_SYS_thr_self	432
#define	IA32_SYS_thr_kill	433
#define	IA32_SYS__umtx_lock	434
#define	IA32_SYS__umtx_unlock	435
#define	IA32_SYS_jail_attach	436
#define	IA32_SYS_MAXSYSCALL	437
