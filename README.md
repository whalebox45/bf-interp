# Brainfuck Experiments (Interpreter Variants)

關於 Brainfuck：[Brainfuck - Wikipedia](https://en.wikipedia.org/wiki/Brainfuck)

這個 repo 放了三個 Brainfuck 實驗性實作，
目的不是「寫一個最快的 BF」，而是逐步整理 **interpreter 結構設計的差異與取捨**。

三支程式不是「好 / 壞」關係，而是**不同設計階段的對照**。

---

## 檔案總覽

* `bf.c`
  最基本、直觀的 Brainfuck interpreter

* `bf2.c`
  引入 **preprocess jump table**，把 `[` `]` 的配對從 runtime 移走

* `bf3.c`
  進一步重構成 **opcode dispatch table**，用 function pointer 取代大型 `switch`

* `test.bf`
  簡單測試用 Brainfuck 程式（例如 `,[.,]`）

* `hello.bf`
  輸出 'HELLO, WORLD'

* `Makefile`
  使用 pattern rule，支援 `make <name>` 直接編譯 `<name>.c`

---

## bf.c：最直接的 Interpreter

`bf.c` 是「沒有花樣」的版本：

* 使用 `switch (opcode)` 處理指令
* runtime 直接掃描 `[` `]`
* 全域狀態（tape / ptr）
* 重點是 **語意清楚、好讀**

這一版適合確認：

* Brainfuck 的 I/O 行為
* EOF → cell = 0 的語意
* tape overflow / pointer wrap 的處理方式

---

## bf2.c：Preprocess Jump Table

`bf2.c` 的主要改動只有一個，但很關鍵：

👉 **在執行前先建立 jump table，配對所有 `[` `]`**

特點：

* preprocess 階段用 stack 建立 `jump[]`
* runtime 中：

  * `[` / `]` 都是 O(1) 跳轉
* 程式控制流變得明確
* interpreter loop 更乾淨

這一版的重點不是效能，而是：

> 把「結構性問題」從 runtime 移到 preprocess

---

## bf3.c：Opcode Dispatch Table

`bf3.c` 進一步把 interpreter 的核心 loop 重構成 **opcode dispatch table**：

* 使用 function pointer 對應 opcode
* 每個 Brainfuck 指令都有自己的 handler
* 非 BF 字元自然被忽略（noop 或 NULL）

概念上從：

```c
switch (opcode) { ... }
```

變成：

```c
dispatch[opcode](ctx, &pc, jump);
```

這一版的目標是：

* 讓 interpreter loop 成為「指令派發器」
* 把語意集中在 handler
* 為之後改成 compiler / IR 做結構鋪路

---

## 關於 computed goto

我**知道** GNU C / Clang 支援 **computed goto**，
也知道它在 interpreter 中可以比 function pointer 更快。

但這個 repo **刻意沒有使用 computed goto**，原因很單純：

* 目前是直譯器，不是效能瓶頸
* I/O 本身遠比 dispatch 慢
* computed goto 不是標準 C
* 可讀性與可攜性成本高

如果未來改寫成 **編譯器或 JIT**，
那會是另一個層級的問題，屆時再考慮。

---

## Makefile 使用方式

Makefile 使用 pattern rule，支援：

```bash
make bf
make bf2
make bf3
```

對應行為是：

```bash
gcc bf.c  -o bf
gcc bf2.c -o bf2
gcc bf3.c -o bf3
```

不需要為每個檔案手寫 target。

清理可自行刪除產生的 binary，
或依需求擴充 `clean` 規則。

---

## 結語

這三支程式不是為了「證明哪個比較快」，
而是用來對照：

* switch-based interpreter
* jump-table-based interpreter
* dispatch-table-based interpreter

如果你對 Brainfuck 有興趣，
真正有價值的不是語言本身，而是這些結構選擇。
