extern crate hex;
extern crate libc;

mod ffi;
mod qtum;

use qtum::*;

fn init(ctx: Context) -> Result<(), Error> {
    return ctx.put("owner", ctx.address.data.as_ref());
}

fn handle(ctx: Context) -> Result<(), Error> {
    match ctx.get("owner") {
        Ok(data) => {
            let owner = Address { data: data };
            println!("owner: {:?}", owner);
            return Ok(());
        },
        Err(err) => {
            return Err(err);
        }
    }
}

fn run() -> Result<(), Error> {
    let ctx = Context::open()?;

    println!("{:?}", ctx);

    match ctx.action {
        Action::InitAction => init(ctx),
        Action::CallAction => handle(ctx),
    }
}

fn main() {
    if let Err(err) = run() {
        println!("err: {:?}", err);
    }

    unsafe {
        ffi::qtum_print_version();
    }
}
