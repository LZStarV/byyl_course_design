// Demo: add function and basic conditional
package main

// add returns the sum of two integers
func add(a int, b int) int {
    return a + b
}

func main() {
    s := add(1, 2)
    // Guard: ensure s is positive
    if s > 0 {
        // no-op
    }
}
