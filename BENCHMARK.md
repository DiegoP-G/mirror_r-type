# **R-Type Benchmark**
---
### **Benchmark criteria table**

| Criteria | C++ | C | Haskell | Python |
| :--- | :--- | :--- | :--- | :--- |
| Performance | :star::star::star::star: | :star::star::star::star: | :star::star: | :star: |
| Memory control | :star::star::star::star: | :star::star::star::star: | :star: | :star: |
| Cross-platform | :star::star::star::star: | :star::star::star::star: | :star::star: | :star::star::star: |
| Concurrency/Multi-threading | :star::star::star::star: | :star::star: | :star::star: | :star: |
|Networking capability | :star::star::star::star: | :star::star::star: | :star: | :star::star: |
| ECS and game engine | :star::star::star: | :star: | :star: | :star: |
---
### **Description of criteria**
**1. Performance**
+ **C++** and **C** both compile to efficient native code, enabling the real-time updates needed for rendering, physics, and networked gameplay.
+ **Haskell** and **Python** suffer from runtime overheads, unsuitable for 60+ FPS and low-latency networking.

**2. Memory control**
+ **C++** and **C** give precise control over memory usage, critical for managing entities, bullets, players, and network buffers without wasted allocations.
+ **Haskell** and **Python** rely on garbage collection which can introduce unpredictable pauses.

**3. Cross-platform**
+ **C++** and **C** are universally portable across Windows and Linux.
+ **Haskell** can be compiled cross-platform but lacks widespread support in game contexts.
+ **Python** is portable but introduces runtime dependencies issues.

**4. Concurrency/Multi-threading**
+ **C++** provides modern threading tools that are efficient and portable, useful for separating rendering, networking and gameplay logic.
+ **C** has threads too, but only via system-specific APIs (e.g. pthreads), making code harder to maintain.
+ **Haskell** supports concurrency but with runtime unpredictability.
+ **Python** is limited by the GIL, preventing true parallel execution.

**5. Networking capability**
+ **C++** provides direct access to low-level sockets (via system calls), efficient data handling, and precise control over serialization/deserialization: perfect for building a custom multiplayer engine.
+ **C** has similar low-level access, but with less abstraction, requiring more boilerplate code.
+ **Haskell** provides networking features, but its high-level runtime and lazy evalutation make real-time, low-latency communication difficult.
+ **Python** provides easy socket programming, but not performant enough for high-frequency packet exchange.

**6. Maintainability for ECS and game engine**
+ **C++** supports modular designs with templates, classes and RAII: all beneficial for ECS and engine patterns.
+ **C** forces verbose boilerplate and lacks abstractions for large architectures.
+ **Haskell** is elegant for pure functions but akward for state-heavy architectures like games.
+ **Python** is easy to maintain for small projects but unsuitable for a performant, scalable engine.
---

### **Conclusion**
We chose **C++** because it strikes the right balance:
+ The performance of **C**
+ With better abstractions and maintainability
+ While avoiding the impracticalities of **Haskell** and **Python** for a real-time, networked ECS-based game.
---