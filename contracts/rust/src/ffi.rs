use libc;

use std;
use std::os::raw::c_char;

#[link(name = "qtum")]
#[link(name = "leveldb")]
extern "C" {
    pub fn qtum_print_version();
    pub fn qtum_context_open(
        argc: i32,
        argv: *const *const c_char,
        err: *mut *const qtum_err,
    ) -> *mut qtum_ctx;
    pub fn qtum_context_close(ctx: *mut qtum_ctx);
    pub fn qtum_err_free(err: *const qtum_err);

    pub fn qtum_put(
        ctx: *const qtum_ctx,
        key: *const u8,
        keylen: libc::size_t,
        data: *const u8,
        datalen: libc::size_t,
        err: *mut *const qtum_err,
    );

    pub fn qtum_get(
        ctx: *const qtum_ctx,
        key: *const u8,
        keylen: libc::size_t,
        retlen: *mut libc::size_t,
        err: *mut *const qtum_err,
    ) -> *const u8;

    pub fn qtum_free_get_data(ptr: *const u8);
}

#[repr(C)]
pub struct qtum_err {
    pub code: u32,
    pub message: *mut c_char,
}

impl Default for qtum_err {
    fn default() -> qtum_err {
        qtum_err {
            code: 0,
            message: std::ptr::null_mut(),
        }
    }
}

#[repr(i32)]
#[derive(Debug, Clone, Copy)]
pub enum Action {
    #[allow(dead_code)]
    InitAction,
    #[allow(dead_code)]
    CallAction,
}

#[repr(C)]
pub struct qtum_ctx {
    pub action: Action,
    pub address: [u8; 20],
    pub sender: [u8; 20],
}



impl qtum_ctx {
    // fn open() -> {

    // }
}
