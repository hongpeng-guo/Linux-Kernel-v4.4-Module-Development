#define pr_fmt(fmt) "cs423_mp4: " fmt

#include <linux/lsm_hooks.h>
#include <linux/security.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/cred.h>
#include <linux/dcache.h>
#include <linux/binfmts.h>
#include "mp4_given.h"

/**
 * get_inode_sid - Get the inode mp4 security label id
 *
 * @inode: the input inode
 *
 * @return the inode's security id if found.
 *
 */
static int get_inode_sid(struct inode *inode)
{
	/*
	 * Add your code here
	 * ...
    */
    int vallen, sid;
    char* val;

	struct dentry *d_dentry = NULL;
    d_dentry = d_find_alias(inode);
    if (! d_dentry){
        return 0;
    } 
    vallen = inode -> i_op -> getxattr(d_dentry, XATTR_NAME_MP4, NULL, 0);
	if (vallen < 0){
		pr_info("cred cxt error!\n");
		dput(d_dentry);
		return vallen;
	}
	val = kmalloc(vallen+1, GFP_KERNEL);
	vallen = inode -> i_op -> getxattr(d_dentry, XATTR_NAME_MP4, val, vallen);
	if (vallen < 0){
		pr_info("cred cxt error!\n");
		dput(d_dentry);
		kfree(val);
		return vallen;
	}
	sid = __cred_ctx_to_sid(val);
	dput(d_dentry);
	kfree(val);
	
	return sid;
}

/**
 * mp4_bprm_set_creds - Set the credentials for a new task
 *
 * @bprm: The linux binary preparation structure
 *
 * returns 0 on success.
 */
static int mp4_bprm_set_creds(struct linux_binprm *bprm)
{
	/*
	 * Add your code here
	 * ...
	 */
    int sid;
    struct mp4_security *msec = bprm -> cred -> security;
    struct inode *inode = file_inode(bprm->file);
    sid = get_inode_sid(inode);
    if (sid >= 0)
        msec -> mp4_flags = sid;
	return 0;
}

/**
 * mp4_cred_alloc_blank - Allocate a blank mp4 security label
 *
 * @cred: the new credentials
 * @gfp: the atomicity of the memory allocation
 *
 */
static int mp4_cred_alloc_blank(struct cred *cred, gfp_t gfp)
{
	/*
	 * Add your code here
	 * ...
	 */
    struct mp4_security *msec;
    msec = (struct mp4_security*) kmalloc(sizeof(struct mp4_security), gfp);
    if (!msec)
        return -ENOMEM;
    msec -> mp4_flags = MP4_NO_ACCESS;
    cred -> security = msec;
	return 0;
}


/**
 * mp4_cred_free - Free a created security label
 *
 * @cred: the credentials struct
 *
 */
static void mp4_cred_free(struct cred *cred)
{
	/*
	 * Add your code here
	 * ...
	 */
    struct mp4_security *msec = cred -> security;
    cred -> security = NULL;
    if (msec) kfree(msec);
}

/**
 * mp4_cred_prepare - Prepare new credentials for modification
 *
 * @new: the new credentials
 * @old: the old credentials
 * @gfp: the atomicity of the memory allocation
 *
 */
static int mp4_cred_prepare(struct cred *new, const struct cred *old,
			    gfp_t gfp)
{       
    const struct mp4_security *old_msec;
    struct mp4_security *msec;

    old_msec = old->security;

	if (!old_tsec) {
		mp4_cred_alloc_blank(new,gfp);
		return 0;
    }
       
	msec = kmemdup(old_tsec, sizeof(struct mp4_security), gfp);
    if (!tsec)
        return -ENOMEM;

    new->security = msec;
	return 0;
}

/**
 * mp4_inode_init_security - Set the security attribute of a newly created inode
 *
 * @inode: the newly created inode
 * @dir: the containing directory
 * @qstr: unused
 * @name: where to put the attribute name
 * @value: where to put the attribute value
 * @len: where to put the length of the attribute
 *
 * returns 0 if all goes well, -ENOMEM if no memory, -EOPNOTSUPP to skip
 *
 */
static int mp4_inode_init_security(struct inode *inode, struct inode *dir,
				   const struct qstr *qstr,
				   const char **name, void **value, size_t *len)
{
	/*
	 * Add your code here
	 * ...
	 */
	int proc_sid;
	struct mp4_security * cs = current_security();
	proc_sid = cs -> mp4_flags;

    if (proc_sid != MP4_TARGET_SID)
        return -EOPNOTSUPP;
    
    * name = XATTR_MP4_SUFFIX;
 
	if (S_ISREG(inode->i_mode)) {
		*value = kstrdup("read-write", GFP_NOFS);
		*len = strlen("read-write");
	}
    else if (S_ISDIR(inode->i_mode)) {
		*value = kstrdup("dir-write", GFP_NOFS);
		*len = strlen("dir-write");
	}

	if (*value == NULL)
		return -ENOMEM;
	return 0;
}

/**
 * mp4_has_permission - Check if subject has permission to an object
 *
 * @ssid: the subject's security id
 * @osid: the object's security id
 * @mask: the operation mask
 *
 * returns 0 is access granter, -EACCES otherwise
 *
 */
static int mp4_has_permission(int ssid, int osid, int mask)
{
	/*
	 * Add your code here
	 * ...
	 */
	int o_mask = 0;

    if ( ssid == MP4_TARGET_SID ) {
        if (osid == MP4_READ_OBJ)
		    o_mask = MAY_READ | MAY_OPEN | MAY_ACCESS;
        else if (osid == MP4_READ_WRITE)
		    o_mask = MAY_READ | MAY_WRITE | MAY_APPEND | MAY_OPEN | MAY_ACCESS;
        else if (osid == MP4_WRITE_OBJ)
		    o_mask = MAY_WRITE | MAY_APPEND | MAY_OPEN | MAY_ACCESS;
        else if (osid == MP4_EXEC_OBJ)
		    o_mask = MAY_READ | MAY_EXEC | MAY_OPEN | MAY_ACCESS;
    	else if (osid == MP4_READ_DIR)
		    o_mask = MAY_READ | MAY_EXEC | MAY_ACCESS | MAY_CHDIR | MAY_OPEN;
    	else if (osid == MP4_RW_DIR)
		    o_mask = MAY_READ | MAY_EXEC | MAY_ACCESS | MAY_CHDIR | MAY_WRITE | MAY_APPEND | MAY_OPEN;
    } else {
        if (osid == MP4_READ_OBJ || osid == MP4_READ_WRITE || osid == MP4_WRITE_OBJ)
		    o_mask = MAY_READ | MAY_OPEN | MAY_ACCESS;
        else if (osid == MP4_NO_ACCESS)
		    o_mask = MAY_ACCESS;
        else if (osid == MP4_EXEC_OBJ)
		    o_mask = MAY_READ | MAY_EXEC | MAY_OPEN | MAY_ACCESS;
    	else if (osid == MP4_READ_DIR || osid == MP4_RW_DIR)
		return 0;
    }
    if (mask > o_mask) {
	    return -EACCES;
	}
	return 0;
}

/**
 * mp4_inode_permission - Check permission for an inode being opened
 *
 * @inode: the inode in question
 * @mask: the access requested
 *
 * This is the important access check hook
 *
 * returns 0 if access is granted, -EACCES otherwise
 *
 */
static int mp4_inode_permission(struct inode *inode, int mask)
{
	/*
	 * Add your code here
	 * ...
	 */
    struct dentry *d_dentry;
    char *pathname, *buf;
    int buflen = 256;
    int skip;

    int ssid = curent_cred() -> security -> mp4_flags;
    int osid = get_inode_sid(inode);

    d_dentry = d_find_alias(inode);
    buf = (char*) kmalloc(buflen, GFP_KERNEL);
    pathname = dentry_path(d_dentry, buf, buflen-1);

    skip = mp4_should_skip_path(pathname);
    kfree(pathname);
    dput(d_dentry);

    if (skip) return 0;
    
    res = mp4_has_permission(ssid, osid, mask);
    return res;    
}


/*
 * This is the list of hooks that we will using for our security module.
 */
static struct security_hook_list mp4_hooks[] = {
	/*
	 * inode function to assign a label and to check permission
	 */
	LSM_HOOK_INIT(inode_init_security, mp4_inode_init_security),
	LSM_HOOK_INIT(inode_permission, mp4_inode_permission),

	/*
	 * setting the credentials subjective security label when laucnhing a
	 * binary
	 */
	LSM_HOOK_INIT(bprm_set_creds, mp4_bprm_set_creds),

	/* credentials handling and preparation */
	LSM_HOOK_INIT(cred_alloc_blank, mp4_cred_alloc_blank),
	LSM_HOOK_INIT(cred_free, mp4_cred_free),
	LSM_HOOK_INIT(cred_prepare, mp4_cred_prepare)
};

static __init int mp4_init(void)
{
	/*
	 * check if mp4 lsm is enabled with boot parameters
	 */
	if (!security_module_enable("mp4"))
		return 0;

	pr_info("mp4 LSM initializing..");

	/*
	 * Register the mp4 hooks with lsm
	 */
	security_add_hooks(mp4_hooks, ARRAY_SIZE(mp4_hooks));

	return 0;
}

/*
 * early registration with the kernel
 */
security_initcall(mp4_init);
