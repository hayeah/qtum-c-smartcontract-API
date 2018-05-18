extern crate hex;

mod ffi;
mod qtum;

use qtum::Context;


fn main() {
    match Context::open() {
        Ok(ctx) => println!("{:?}", ctx),
        Err(err) => println!("err: {:?}", err)
    }

    unsafe {
        ffi::qtum_print_version();
    }
}
