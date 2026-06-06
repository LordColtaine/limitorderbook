# ⚡ Low-Latency NASDAQ ITCH 5.0 Feed Handler

A high-performance, multithreaded C++ market data feed handler and matching engine designed to parse the raw NASDAQ ITCH 5.0 binary protocol. 

This engine simulates a real-world quantitative trading infrastructure by rebuilding the Level 2 Limit Order Book (LOB) in real-time and calculating Order Book Imbalance trading signals. It is built with a strict focus on High-Frequency Trading (HFT) concepts, including zero-allocation critical paths, lock-free concurrency, and Data-Oriented Design (DOD).

## 🚀 Key Features

* **Multithreaded Architecture:** Completely decouples I/O from business logic using a dual-thread setup (Network Producer & Logic Consumer).
* **Lock-Free Queue:** Utilizes a custom Single-Producer, Single-Consumer (SPSC) lock-free ring buffer (std::atomic) for sub-microsecond thread communication without mutex context switching.
* **Dual-Mode Order Book:**
  * **HFT Mode (Array-Backed):** An $O(1)$ flat-array limit order book that eliminates red-black tree heap allocations and CPU cache misses. Sharded to a single equity (e.g., AAPL) for extreme latency reduction.
  * **Data Science Mode (Map-Backed):** An $O(\log N)$ `std::map` implementation capable of ingesting and tracking the entire market (8,000+ equities) simultaneously for macroscopic backtesting.
* **Pre-Allocated Memory:** Uses a contiguous memory `OrderPool` capable of holding millions of active orders, completely bypassing `new`/`malloc` during live market hours.
* **Real-Time Alpha Generation:** Calculates top-of-book imbalance dynamically to identify market micro-structure pressures (buying vs. selling walls).

## 🧠 System Architecture

The engine uses a lock-free pipeline to prevent the operating system from interrupting the critical path.

1. **The Producer Thread:** Reads raw binary packets from the network/disk, performs fast little-endian to big-endian byte swapping, ignores irrelevant packets to save CPU cycles, packs data into an `InternalEvent` struct, and spin-waits to push it to the queue.
2. **The Lock-Free Ring Buffer:** A circular array using strictly fenced `std::atomic<size_t>` pointers. 
3. **The Consumer Thread:** Pops internal structs from the queue, updates the deterministic memory pool, reconstructs the Bid/Ask spread, and calculates quantitative trading signals based on book pressure.

## 🛠️ Build Instructions

This project uses **CMake** for cross-platform building.

```bash
# 1. Clone the repository
git clone [https://github.com/YOUR_USERNAME/YOUR_REPO_NAME.git](https://github.com/YOUR_USERNAME/YOUR_REPO_NAME.git)
cd YOUR_REPO_NAME

# 2. Create a build directory
mkdir build
cd build

# 3. Generate Makefiles and compile
cmake ..
make

# 4. Run the engine
./program
```
(Note: You must supply your own NASDAQ ITCH 5.0 binary data file and place it in the data/ directory, updating the file path in main.cpp accordingly).

## ⚙️ Configuration (the dual-toggle)

In include/limitorderbook.h, you can toggle the engine's architectural behavior based on your hardware limits and data needs:

```C++
// Comment this line out to run the "Data Science" Map mode for all stocks.
// Leave it uncommented to run the "HFT" Array mode for extreme low-latency single-stock tracking.
#define LOW_LATENCY_MODE
```

## 📊 Sample Output

```PlainText
--- Engine Starting: Multithreaded Mode ---

--- Processed 5 Million Network Messages --- | Active Orders in RAM: 324150
[HFT MODE] [AAPL    ] Bid: $320.81 <--> Ask: $320.34
Order Book Imbalance: -0.658

--- Processed 6 Million Network Messages --- | Active Orders in RAM: 410100
[HFT MODE] [AAPL    ] Bid: $320.81 <--> Ask: $320.34
Order Book Imbalance: 0.982
>>> SIGNAL: MASSIVE BUYING PRESSURE! EXECUTING LONG TRADE.

--- File Fully Processed. Signaling Shutdown ---
```

## 📝 Next Steps / Roadmap

* Implement a PortfolioManager class to execute mock virtual trades against the imbalance signal and track PnL.
* Connect the Producer thread to a live UDP network socket (e.g., using libpcap or kernel-bypass like DPDK).
* Implement a TCP order entry gateway (FIX protocol) for outbound simulated routing.
