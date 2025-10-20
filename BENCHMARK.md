# **R-Type Benchmark**
---

## **Main progamming language benchmark**

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

## **Graphical library benchmark**

| Criteria | SFML | SDL2 | Raylib |
| :--- | :--- | :--- | :--- |
| Ease of use | :star::star::star::star: | :star::star: | :star::star::star: |
| C++ integration | :star::star::star::star: | :star::star: | :star::star::star: |
| 2D rendering | :star::star::star: | :star::star::star::star: | :star::star::star::star: |
| Multimedia support (audio, input, network) | :star::star::star::star: | :star::star::star: | :star: |
| Cross-platform compatibility | :star::star::star::star: | :star::star::star::star: | :star::star::star: |
| Documentation & community | :star::star::star::star: | :star::star::star::star: | :star::star: |
---

### **Description of criteria**

**1. Ease of use**
+ **SFML** provides a clean, object-oriented API that’s intuitive and beginner-friendly.
+ **SDL2** is lower-level and requires more boilerplate to perform the same tasks.
+ **Raylib** is simple but more C-oriented and less idiomatic for C++ development.

**2. C++ integration**
+ **SFML** is designed in C++ and fits perfectly into our engine’s OOP and ECS structure.
+ **SDL2** is written in C, requiring wrappers or manual conversions for C++ use.
+ **Raylib** has a C API but provides some C++ bindings.

**3. 2D rendering**
+ **SDL2** and **Raylib** are both highly optimized for 2D rendering.
+ **SFML** is slightly less performant but still sufficient for real-time 2D games like R-Type.

**4. Multimedia support**
+ **SFML** directly supports graphics, audio, input handling, and networking, all in one library.
+ **SDL2** requires additional modules (SDL_image, SDL_ttf, SDL_mixer, etc.).
+ **Raylib** focuses mainly on graphics, offering fewer built-in features.

**5. Cross-platform compatibility**
+ All three libraries support Windows, Linux, and macOS.
+ **SFML** and **SDL2** are more mature and stable across platforms.

**6. Documentation & community**
+ **SFML** and **SDL2** have strong communities, tutorials, and examples.
+ **Raylib** has fewer resources and a smaller ecosystem.
---

### **Conclusion**
We chose **SFML** because it provides the best balance between simplicity, C++ integration, and multimedia support.
+ Our team already had experience with it, allowing us to focus on the engine and gameplay logic rather than setup complexity.
+ For a 2D networked project like R-Type, **SFML** is the most efficient and developer-friendly choice.
---

## **Data compression benchmark**

| Criteria | LZ4 | Zlib | RLE |
| :--- | :--- | :--- | :--- |
| Compression ratio | :star::star: | :star::star::star::star: | :star: |
| Compression speed | :star::star::star::star: | :star::star: | :star::star::star: |
| Decompression speed | :star::star::star::star: | :star::star::star: | :star::star::star:|
| CPU usage | :star::star::star::star: | :star::star: | :star::star::star::star: |
| Implementation complexity | :star::star::star: | :star::star::star: | :star::star: |
---

### **Description of criteria**

**1. Compression ratio**
+ **LZ4** is focused on speed and provides weaker compression (typically 20–30%).
+ **Zlib** achieves a strong balance between compression rate and data integrity, often reducing packet size by 50–70%, which helps lower network bandwidth usage.
+ **RLE (Run-Length Encoding)** only works efficiently on repetitive data, leading to poor results for mixed binary payloads like network packets.

**2. Compression speed**
+ **LZ4** is extremely fast: one of the fastest general-purpose algorithms, ideal for real-time systems.
+ **Zlib** is slower due to its more advanced deflate algorithm but remains performant at low compression levels.
+ **RLE** is simple and relatively fast, but its speed advantage is often offset by its inefficiency on non-repetitive data.

**3. Decompression speed**
+ **LZ4** and **RLE** decompress extremely fast, which is critical for real-time applications.
+ **Zlib** is slightly slower, but its decompression remains fast enough for packet handling, especially since game packets are small (hundreds of bytes).

**4. CPU usage**
+ **LZ4** uses less CPU than **Zlib**, which is why it’s often used in high-throughput systems (databases, game engines).
+ **Zlib** consumes more CPU time during compression, but the cost is acceptable given the improved ratio and bandwidth savings.
+ **RLE** uses minimal CPU but provides minimal benefits.

**5. Implementation complexity**
+ Both **Zlib** and **LZ4** provide mature, cross-platform libraries and C APIs.
+ **RLE** is easy to implement manually but unsuitable for general-purpose binary compression.
---

### **Conclusion**
We chose **Zlib** because it offers the best compromise between compression efficiency and speed for our network packets:
+ While **LZ4** is faster, its weaker compression led to larger payloads and no real gain in performance.
+ **RLE** was discarded as it only works well on highly repetitive data.
+ **Zlib** gives us smaller packets, reliable results, and consistent performance across all types of game data.
---
