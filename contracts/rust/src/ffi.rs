use std;
use std::os::raw::c_char;

#[link(name = "qtum")]
#[link(name = "leveldb")]
extern "C" {
  pub fn qtum_print_version();
  pub fn qtum_context_open(argc: i32, argv: *const *const c_char, err: *mut *mut qtum_err) -> *mut qtum_ctx;
  pub fn qtum_context_close(ctx: *mut qtum_ctx);
  pub fn qtum_err_free(err: *mut qtum_err);
}

#[repr(C)]
pub struct qtum_err {
  pub code: u32,
  pub message: *mut u8,
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
