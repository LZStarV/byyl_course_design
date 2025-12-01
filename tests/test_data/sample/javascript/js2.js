// Sample JS 2
// Demo: import, class, template string
import { readFile } from 'fs';
class Greeter {
  constructor(name) { this.name = name; }
  greet() {
    // Use template string interpolation
    return `Hello, ${this.name}`;
  }
}
const g = new Greeter('World');
if (g.greet() === 'Hello, World') {
  write(1 + 2);
}
