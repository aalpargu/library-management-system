# Kütüphane Yönetim Sistemi

Öğrenci, kitap, yazar ve ödünç/iade kayıtlarını yöneten C konsol uygulaması. Bağlı listeler, dinamik bellek yönetimi, fonksiyon işaretçileri ve dosyada kalıcılık üzerine akademik bir çalışma.

[English](README.md)

Öğrenciler çift yönlü; kitaplar, yazarlar, kitap kopyaları ve ödünç kayıtları tek yönlü bağlı listelerde tutulur. Kitap-yazar ilişkisi dinamik diziyle saklanır. Ortak CSV okuma/yazma döngüleri kayıt türüne göre callback alır. Kod `src/kutuphane.c` içindedir.

## Derleme

GCC gibi bir C derleyicisiyle:

```sh
gcc -std=c99 -Wall -Wextra src/kutuphane.c -o kutuphane
./kutuphane
```

CSV dosyaları çalışma klasörüne göre okunup yazılır; dosya adları kaynak dosyanın başında tanımlıdır. **Eski README'de sözü edilen örnek CSV/data klasörü depoda bulunmuyor.** Mevcut kayıtlarla çalışmadan önce bunların yedeğini alın.

Menü öğrenci, kitap ve yazar işlemlerini; ödünç/iade, gecikme tespiti ve puan tabanlı cezaları kapsar.

Tek kaynak dosyalı bir ders uygulamasıdır. Otomatik regresyon testi bulunmaz. 30 Ekim 2026 incelemesinde kaynak kod okundu; C uygulaması derlenip etkileşimli olarak test edilmedi. Yukarıdaki komut, belgelenen derleme adımıdır.
