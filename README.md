# 📚 Library Management System (Kütüphane Otomasyon Sistemi)

A full-featured, console-based library management system built in C. Manages students, books, authors, and borrowing/return operations with CSV-based data persistence.

> **Kütüphane Otomasyon Sistemi** — Öğrenci, kitap, yazar ve ödünç/iade işlemlerini yöneten, CSV tabanlı veri kalıcılığına sahip konsol uygulaması.

---

## ✨ Features

- **Student Management** — Add, edit, delete students with unique 8-digit IDs
- **Book Management** — Track books by ISBN with multiple copy support
- **Author Management** — Maintain author records with auto-incrementing IDs
- **Borrowing System** — Borrow and return book copies with date tracking
- **Overdue Detection** — Automatic detection of overdue books (>15 days)
- **Penalty System** — Score-based penalty system for late returns
- **Data Persistence** — All data stored in CSV files, loaded on startup and saved on changes

## 🏗️ Technical Highlights

| Feature | Implementation |
|---|---|
| **Singly Linked List** | Authors, Books, Book Copies, Borrow Records |
| **Doubly Linked List** | Students (enables O(1) deletion) |
| **Function Pointers** | Generic CSV reader/writer and record display callbacks |
| **Dynamic Memory** | `malloc`/`realloc`/`free` with full cleanup on exit |
| **Sorted Insertion** | Authors sorted by ID, Books sorted by name (then ISBN) |
| **Relational Data** | Book-Author many-to-many relationship via dynamic array |

## 📁 Project Structure

```
├── src/
│   └── kutuphane.c          # Main source file (~1100 lines)
├── data/
│   ├── Yazarlar.csv          # Authors data
│   ├── Ogrenciler.csv        # Students data
│   ├── Kitaplar.csv          # Books data
│   ├── KitapOrnekleri.csv    # Book copies data
│   ├── KitapYazar.csv        # Book-Author relationships
│   └── KitapOdunc.csv        # Borrowing records
└── README.md
```

## 🚀 Build & Run

### Compile
```bash
gcc -o kutuphane src/kutuphane.c -Wall -Wextra
```

### Run
```bash
./kutuphane
```

> **Note:** The program expects CSV data files in the working directory. Sample data files are provided in the `data/` folder — copy them to your working directory before running.

## 📊 Data Model

```
┌──────────────┐     ┌──────────────────┐     ┌──────────────┐
│   Yazarlar   │     │  KitapYazar (M:N) │     │   Kitaplar   │
│──────────────│     │──────────────────│     │──────────────│
│ yazarNo (PK) │◄────│ yazarID (FK)     │────►│ isbnNo (PK)  │
│ adi          │     │ kitabinIsbn (FK)  │     │ kitapAdi     │
│ soyadi       │     └──────────────────┘     │ kopyaSayisi  │
└──────────────┘                               └──────┬───────┘
                                                      │ 1:N
                                               ┌──────┴───────┐
┌──────────────┐     ┌──────────────────┐     │ KitapKopyasi │
│  Ogrenciler  │     │  OduncIslemi     │     │──────────────│
│──────────────│     │──────────────────│     │ kopyaEtiketi │
│ ogrNo (PK)   │◄────│ oduncAlanOgrNo   │     │ durumu       │
│ adi          │     │ oduncKitapEtiketi│────►│ (Rafta/OgrNo)│
│ soyadi       │     │ islemTipi (0/1)  │     └──────────────┘
│ puan         │     │ islemTarihi      │
└──────────────┘     └──────────────────┘
```

## 🎯 Menu Structure

```
Main Menu
├── 1. Student Operations
│   ├── Add / Delete / Edit Student
│   ├── View Student Details & Borrow History
│   ├── List Students with Unreturned Books
│   └── List Penalized Students
├── 2. Book Operations
│   ├── Add / Delete / Edit Book
│   ├── View All Books & Copies
│   ├── List Available Books (on shelf)
│   ├── List Overdue Books
│   └── Assign / Update Book Authors
├── 3. Author Operations
│   ├── Add / Delete / Edit Author
│   └── View Author's Books
├── 4. Borrow a Book
├── 5. Return a Book
└── 0. Exit
```

## 🛠️ Tech Stack

- **Language:** C (C99)
- **Data Structures:** Singly & Doubly Linked Lists, Dynamic Arrays
- **Storage:** CSV File I/O
- **Design Patterns:** Callback functions via function pointers for generic operations
