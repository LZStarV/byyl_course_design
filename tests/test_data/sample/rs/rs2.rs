// Demo: add function and print result
fn add(a: i32, b: i32) -> i32 {
    a + b
}

fn main() {
    let s = add(1, 2);
    // Only print positive results
    if s > 0 {
        println!("{}", s);
    }
}
