use ffi::*;

use hex;

use std;
use std::ffi::CString;
use std::os::raw::c_char;

pub use ffi::Action;

#[derive(Debug)]
pub struct Context {
    pub action: Action,
    pub address: Address,
    pub sender: Address,

    // private
    ctx: *mut qtum_ctx,
}

impl Drop for Context {
    fn drop(&mut self) {
        unsafe {
            qtum_context_close(self.ctx);
        }
    }
}

impl Context {
    pub fn open() -> Result<Context, Error> {
        let args : Vec<CString> = std::env::args().map(|str|
            std::ffi::CString::new(str).unwrap()
        ).collect();

        let argps : Vec<*const c_char> = args.iter().map(|s| s.as_ptr() ).collect();

        let mut err: *mut qtum_err = std::ptr::null_mut();

        unsafe {
            let ctx = qtum_context_open(args.len() as i32, argps.as_ptr(), &mut err);
            if !err.is_null() {
                let msg = CString::from_raw((*err).message as *mut i8);

                let rerr = Error{
                    code: (*err).code,
                    message: msg.into_string().unwrap(),
                };

                qtum_err_free(err);
                return Err(rerr);
            }

            return Ok(Context {
                action: (*ctx).action,
                sender: Address::new(&(*ctx).sender),
                address: Address::new(&(*ctx).address),
                ctx: ctx,
            });
        }
    }
}

#[derive(Debug)]
pub struct Error {
    code: u32,
    message: String,
}

pub struct Address {
    data: Vec<u8>,
}

impl Address {
    fn new(data: &[u8; 20]) -> Address {
        Address{data:data.to_vec()}
    }
}

impl std::fmt::Debug for Address {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{}", hex::encode(&self.data))
    }
}