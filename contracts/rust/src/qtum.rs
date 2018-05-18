use ffi::*;

use hex;

use std;
use std::ffi::CString;
use std::os::raw::c_char;

use libc;

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
        let args: Vec<CString> = std::env::args()
            .map(|str| std::ffi::CString::new(str).unwrap())
            .collect();

        let argps: Vec<*const c_char> = args.iter().map(|s| s.as_ptr()).collect();

        let mut err: *const qtum_err = std::ptr::null_mut();

        unsafe {
            let ctx = qtum_context_open(args.len() as i32, argps.as_ptr(), &mut err);
            if !err.is_null() {
                return Err(Error::from_c(err));
            }

            return Ok(Context {
                action: (*ctx).action,
                sender: Address::new(&(*ctx).sender),
                address: Address::new(&(*ctx).address),
                ctx: ctx,
            });
        }
    }

    // can i support &str using Into [u8]?
    pub fn put(&self, key: &str, data: &[u8]) -> Result<(), Error> {
        let mut err: *const qtum_err = std::ptr::null();

        unsafe {
            qtum_put(self.ctx, key.as_ptr(), key.len(), data.as_ptr(), data.len(), &mut err);

            if !err.is_null() {
                return Err(Error::from_c(err));
            }

            return Ok(());
        }
    }

    pub fn get(&self, key: &str) -> Result<Vec<u8>, Error> {
        let mut err: *const qtum_err = std::ptr::null();

        unsafe {
            let mut retlen: libc::size_t = 0;
            let cdata = qtum_get(self.ctx, key.as_ptr(), key.len(), &mut retlen, &mut err);

            if !err.is_null() {
                return Err(Error::from_c(err));
            }


            let mut data: Vec<u8> = Vec::with_capacity(retlen);
            data.set_len(retlen);
            std::ptr::copy(cdata, data.as_mut_ptr(), retlen);

            qtum_free_get_data(cdata);

            return Ok(data);

        }

    }
}

#[derive(Debug)]
pub struct Error {
    code: u32,
    message: String,
}

impl Error {
    fn from_c(err: *const qtum_err) -> Error {
        unsafe {
            let msg = CString::from_raw((*err).message);

            let str = msg.clone().into_string().unwrap();

            let rerr = Error {
                code: (*err).code,
                message: str,
            };

            // release ownership to err->message pointer
            msg.into_raw();

            qtum_err_free(err);

            return rerr;
        }

    }
}

pub struct Address {
    pub data: Vec<u8>,
}

impl Address {
    pub fn new(data: &[u8; 20]) -> Address {
        Address { data: data.to_vec() }
    }
}

impl std::fmt::Debug for Address {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{}", hex::encode(&self.data))
    }
}
