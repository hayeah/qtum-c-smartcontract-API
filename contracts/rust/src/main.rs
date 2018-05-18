extern crate hex;

mod ffi;
mod qtum;

use qtum::*;

fn init(ctx: Context) -> Result<(), Error> {
    Ok(())
}

fn handle(ctx: Context) -> Result<(), Error> {
    Ok(())
}

fn run() -> Result<(), Error> {
    let ctx = Context::open()?;

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
