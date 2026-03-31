#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_AD_UZUNLUK 50
#define MAX_SOYAD_UZUNLUK 50
#define ISBN_UZUNLUK 14
#define OGRENCI_NO_UZUNLUK 9
#define KITAP_ETIKET_UZUNLUK 30
#define TARIH_UZUNLUK 11
#define MAX_ODUNC_SURESI 15

// Verileri aldýðýmýz Dosya adlarý
#define YAZARLAR_DOSYASI "Yazarlar.csv"
#define OGRENCILER_DOSYASI "Ogrenciler.csv"
#define KITAPLAR_DOSYASI "Kitaplar.csv"
#define KITAP_KOPYALARI_DOSYASI "KitapOrnekleri.csv"
#define KITAP_YAZAR_DOSYASI "KitapYazar.csv"
#define ODUNC_KAYITLARI_DOSYASI "KitapOdunc.csv"

// Yazarýn bilgilerini tutan yapý.
typedef struct YazarBilgisi {
    int yazarNo;
    char adi[MAX_AD_UZUNLUK];
    char soyadi[MAX_SOYAD_UZUNLUK];
    struct YazarBilgisi* sonrakiYazar;
} YazarBilgisi;

// Öðrencinin bilgilerini tutan yapý.
typedef struct OgrenciBilgisi {
    char ogrNo[OGRENCI_NO_UZUNLUK];
    char adi[MAX_AD_UZUNLUK];
    char soyadi[MAX_SOYAD_UZUNLUK];
    int puan;
    struct OgrenciBilgisi* sonrakiOgrenci;
    struct OgrenciBilgisi* oncekiOgrenci;
} OgrenciBilgisi;

// Kitabýn her bir kopyasýnýn bilgilerini tutuyor. ISBN_KopyaNo þeklinde etiketi var.
// Durumu "Rafta" ya da öðrenci numarasý olabilir.
typedef struct KitapKopyasi {
    char kopyaEtiketi[KITAP_ETIKET_UZUNLUK];
    char durumu[OGRENCI_NO_UZUNLUK];
    struct KitapKopyasi* sonrakiKopya;
} KitapKopyasi;

// Kitabýn genel bilgileri. ISBN, adý ve kaç kopyasý olduðu. Kopyalar da baðlý liste.
typedef struct KitapBilgisi {
    char isbnNo[ISBN_UZUNLUK];
    char kitapAdi[100];
    int kopyaSayisi;
    KitapKopyasi* kitapKopyalari;
    struct KitapBilgisi* sonrakiKitap;
} KitapBilgisi;

// Hangi kitabýn hangi yazara ait olduðunu tutuyor.
typedef struct KitapYazarIliskisi {
    char kitabinIsbn[ISBN_UZUNLUK];
    int yazarID;
} KitapYazarIliskisi;

// Ödünç alma veya iade etme iþlemlerinin kaydýný tutuyor.
typedef struct OduncIslemi {
    char oduncKitapEtiketi[KITAP_ETIKET_UZUNLUK];
    char oduncAlanOgrNo[OGRENCI_NO_UZUNLUK];
    int islemTipi; // 0: Odunc Alma, 1: Iade Etme
    char islemTarihi[TARIH_UZUNLUK];
    struct OduncIslemi* sonrakiOduncIslemi;
} OduncIslemi;

// Bütün kütüphane verilerini bir arada tutan ana yapý.
typedef struct KutuphaneVerisi {
    YazarBilgisi* ilkYazar;
    OgrenciBilgisi* ilkOgrenci;
    KitapBilgisi* ilkKitap;
    KitapYazarIliskisi* kitapYazarIliskileri;
    int kitapYazarAdedi;
    OduncIslemi* ilkOduncKaydi;
    int siradakiYazarNo;
} KutuphaneVerisi;


// Fonksiyon iþaretçileri. Bunlar sayesinde bazý fonksiyonlarý daha genel yazabiliyoruz.
// Mesela CSV'den satýr okuyup iþleyen fonksiyon.
typedef void (*SatirIsleyiciFonksiyon)(KutuphaneVerisi* veri, char* satir);
// Bir baðlý liste düðümünü dosyaya yazan fonksiyon.
typedef int (*DugumYazdiriciFonksiyon)(FILE* fp, void* dugum);
// Bir baðlý listede sonraki düðümü getiren fonksiyon.
typedef void* (*SonrakiDugumAlFonksiyonu)(void* dugum);
// Tek bir kaydý ekrana yazdýran fonksiyon.
typedef void (*TekKayitGosterFonksiyonu)(void* kayit_dugumu);

// Yardýmcý fonksiyonlarýn prototipleri. Bunlar ufak tefek iþler yapýyor.
int satirAl(FILE* fp, char* tampon, size_t tampon_boyutu);
void tarihiParcala(const char* tarih_str, int* gun, int* ay, int* yil);
int gunFarkiniHesapla(const char* tarih1_str, const char* tarih2_str);
void bugununTarihiniAl(char* tarih_tamponu);
void bosluklariSil(char *str);

// Genel dosya okuma, yazma ve listeleme fonksiyonlarý.
void genelDosyaOku(KutuphaneVerisi* veri, const char* dosyaAdi, const char* varlikAdi, SatirIsleyiciFonksiyon satirIsle);
void genelDosyaYaz(const char* dosyaAdi, const char* varlikAdi, void* ilkDugum, DugumYazdiriciFonksiyon dugumYazdir, SonrakiDugumAlFonksiyonu sonrakiDugumuAl);
void genelKayitListele(void* ilkDugum,
                       const char* listeBasligi,
                       const char* sutunBasliklari,
                       TekKayitGosterFonksiyonu tekKayitGoster,
                       SonrakiDugumAlFonksiyonu sonrakiKaydiAl,
                       const char* bulunamadiMesaji,
                       const char* ayiracCizgisi);


// Verileri dosyalardan yükleyen ve dosyalara kaydeden fonksiyonlar.
void yazarlariDosyadanAl(KutuphaneVerisi* veri);
void ogrencileriDosyadanAl(KutuphaneVerisi* veri);
void kitapAnaBilgileriniDosyadanAl(KutuphaneVerisi* veri);
void oduncKayitlariniDosyadanAl(KutuphaneVerisi* veri);
void yazarlariDosyayaYaz(KutuphaneVerisi* veri);
void ogrencileriDosyayaYaz(KutuphaneVerisi* veri);
void kitapAnaBilgileriniDosyayaYaz(KutuphaneVerisi* veri);
void oduncKayitlariniDosyayaYaz(KutuphaneVerisi* veri);

void kitapKopyalariniYukleVeIliskilendir(KutuphaneVerisi* veri);
void kitapKopyalariniDosyayaYaz(KutuphaneVerisi* veri);
void kitapYazarIliskileriniDosyadanAl(KutuphaneVerisi* veri);
void kitapYazarIliskileriniDosyayaYaz(KutuphaneVerisi* veri);


// Yazar iþlemleri için özel geri çaðýrma fonksiyonlarý.
void yazarSatiriniIsle(KutuphaneVerisi* veri, char* satir);
int yazarDugumunuYazdir(FILE* fp, void* dugum);
void* sonrakiYazarDugumunuAl(void* dugum);
void tekYazarGoster(void* kayit);

// Öðrenci iþlemleri için özel geri çaðýrma fonksiyonlarý.
void ogrenciSatiriniIsle(KutuphaneVerisi* veri, char* satir);
int ogrenciDugumunuYazdir(FILE* fp, void* dugum);
void* sonrakiOgrenciDugumunuAl(void* dugum);
void tekOgrenciGoster(void* kayit);

// Kitap ana bilgi iþlemleri için özel geri çaðýrma fonksiyonlarý.
void kitapAnaBilgiSatiriniIsle(KutuphaneVerisi* veri, char* satir);
int kitapAnaBilgiDugumunuYazdir(FILE* fp, void* dugum);
void* sonrakiKitapAnaBilgiDugumunuAl(void* dugum);

// Ödünç iþlemleri için özel geri çaðýrma fonksiyonlarý.
void oduncKaydiSatiriniIsle(KutuphaneVerisi* veri, char* satir);
int oduncKaydiDugumunuYazdir(FILE* fp, void* dugum);
void* sonrakiOduncKaydiDugumunuAl(void* dugum);


// Yazarla ilgili yapýlacak iþlemlerin fonksiyonlarý.
YazarBilgisi* yeniYazarDugumuOlustur(int id, const char* ad, const char* soyad);
void yazariListeyeEkle(KutuphaneVerisi* veri, YazarBilgisi* yeniYazar);
void yazarEkleArayuz(KutuphaneVerisi* veri);
YazarBilgisi* yazarBulNoIle(KutuphaneVerisi* veri, int id);
void yazarSilArayuz(KutuphaneVerisi* veri);
void yazarDuzenleArayuz(KutuphaneVerisi* veri);
void butunYazarlariGoster(KutuphaneVerisi* veri);
void yazar_silindiginde_kitap_yazari_guncelle(KutuphaneVerisi* veri, int yazarID);
void yazarinKitaplariniListele(KutuphaneVerisi* veri, int yazarID);
void yazarinKitaplariniGosterArayuz(KutuphaneVerisi* veri);

// Öðrenciyle ilgili yapýlacak iþlemlerin fonksiyonlarý.
OgrenciBilgisi* ogrenciOlustur(const char* ogrenciNumarasi, const char* ad, const char* soyad, int puan);
int ogrenciyiListeyeEkle(KutuphaneVerisi* veri, OgrenciBilgisi* yeniOgrenci);
void ogrenciEkleArayuz(KutuphaneVerisi* veri);
OgrenciBilgisi* ogrenciBulNoIle(KutuphaneVerisi* veri, const char* ogrenciNumarasi);
void ogrenciSilArayuz(KutuphaneVerisi* veri);
void ogrenciDuzenleArayuz(KutuphaneVerisi* veri);
void butunOgrencileriGoster(KutuphaneVerisi* veri);
void ogrenciDetaylariniGosterArayuz(KutuphaneVerisi* veri);
void iadeEtmeyenOgrencileriGoster(KutuphaneVerisi* veri);
void cezaliOgrencileriGoster(KutuphaneVerisi* veri);
void ogrencininOduncGecmisiniGoster(KutuphaneVerisi* veri, const char* ogrenciID);

// Kitapla ilgili yapýlacak iþlemlerin fonksiyonlarý.
KitapBilgisi* kitapOlustur(const char* isbn, const char* ad, int adet_kullanilmiyor);
KitapKopyasi* yeniKitapKopyasiOlustur(const char* kitap_isbn, int kopya_numarasi, const char* durum);
void kitabiListeyeEkle(KutuphaneVerisi* veri, KitapBilgisi* yeniKitap);
void kitapEkleArayuz(KutuphaneVerisi* veri);
KitapBilgisi* kitapBulIsbnIle(KutuphaneVerisi* veri, const char* isbn);
KitapKopyasi* raftaUygunKopyaBul(KitapBilgisi* kitap);
void kitapSilArayuz(KutuphaneVerisi* veri);
void kitapDuzenleArayuz(KutuphaneVerisi* veri);
void butunKitapVeKopyalariGoster(KutuphaneVerisi* veri);
void raftakiKitaplariGoster(KutuphaneVerisi* veri);
void gecikenKitaplariGoster(KutuphaneVerisi* veri);

// Kitap-Yazar iliþkileriyle ilgili fonksiyonlar.
void kitabaYazarAta(KutuphaneVerisi* veri, const char* kitapISBN, int yazarID);
void kitabaYazarAtaArayuz(KutuphaneVerisi* veri);
void kitapYazarGuncelleArayuz(KutuphaneVerisi* veri);
void kitabinYazarlariniListele(KutuphaneVerisi* veri, const char* kitapISBN);

// Ödünç ve iade iþlemleriyle ilgili fonksiyonlar.
OduncIslemi* yeniOduncKaydiOlustur(const char* kitapEtiketNo, const char* ogrenciID, int islemTuru, const char* tarih);
void oduncKaydiniListeyeEkle(KutuphaneVerisi* veri, OduncIslemi* yeniKayit);
void kitapOduncAlArayuz(KutuphaneVerisi* veri);
void kitapIadeEtArayuz(KutuphaneVerisi* veri);

// Kullanýcý arayüzü menülerini gösteren fonksiyonlar.
void ogrenciIslemleriMenusu(KutuphaneVerisi* veri);
void kitapIslemleriMenusu(KutuphaneVerisi* veri);
void yazarIslemleriMenusu(KutuphaneVerisi* veri);
void anaMenuGoster(KutuphaneVerisi* veri);

int main() {
    KutuphaneVerisi kutuphaneVeri; // Bütün verilerimizi bu deðiþkende tutacaðýz.

    kutuphaneVeri.ilkYazar = NULL;
    kutuphaneVeri.ilkOgrenci = NULL;
    kutuphaneVeri.ilkKitap = NULL;
    kutuphaneVeri.kitapYazarIliskileri = NULL;
    kutuphaneVeri.kitapYazarAdedi = 0;
    kutuphaneVeri.ilkOduncKaydi = NULL;
    kutuphaneVeri.siradakiYazarNo = 1; // Yeni yazar eklenince bu numara artýyor

    // Dosyalardaki verileri okuyup programýn belleðine yüklüyoruz.
    yazarlariDosyadanAl(&kutuphaneVeri);
    ogrencileriDosyadanAl(&kutuphaneVeri);
    kitapAnaBilgileriniDosyadanAl(&kutuphaneVeri);
    kitapKopyalariniYukleVeIliskilendir(&kutuphaneVeri);
    kitapYazarIliskileriniDosyadanAl(&kutuphaneVeri);
    oduncKayitlariniDosyadanAl(&kutuphaneVeri);

    anaMenuGoster(&kutuphaneVeri);

    // Program kapanýrken bellekte ayýrdýðýmýz yerleri geri veriyoruz.
    YazarBilgisi* mevcutYazar = kutuphaneVeri.ilkYazar;
    YazarBilgisi* sonrakiYazar;
    while (mevcutYazar != NULL) {
        sonrakiYazar = mevcutYazar->sonrakiYazar;
        free(mevcutYazar);
        mevcutYazar = sonrakiYazar;
    }
    kutuphaneVeri.ilkYazar = NULL;

    OgrenciBilgisi* mevcutOgrenci = kutuphaneVeri.ilkOgrenci;
    OgrenciBilgisi* sonrakiOgrenci;
    while (mevcutOgrenci != NULL) {
        sonrakiOgrenci = mevcutOgrenci->sonrakiOgrenci;
        free(mevcutOgrenci);
        mevcutOgrenci = sonrakiOgrenci;
    }
    kutuphaneVeri.ilkOgrenci = NULL;

    KitapBilgisi* mevcutKitap = kutuphaneVeri.ilkKitap;
    KitapBilgisi* sonrakiKitap;
    KitapKopyasi* mevcutKopya, *sonrakiKopya;
    while (mevcutKitap != NULL) {
        mevcutKopya = mevcutKitap->kitapKopyalari;
        while(mevcutKopya != NULL) {
            sonrakiKopya = mevcutKopya->sonrakiKopya;
            free(mevcutKopya);
            mevcutKopya = sonrakiKopya;
        }
        sonrakiKitap = mevcutKitap->sonrakiKitap;
        free(mevcutKitap);
        mevcutKitap = sonrakiKitap;
    }
    kutuphaneVeri.ilkKitap = NULL;

    free(kutuphaneVeri.kitapYazarIliskileri);
    kutuphaneVeri.kitapYazarIliskileri = NULL;
    kutuphaneVeri.kitapYazarAdedi = 0;

    OduncIslemi* mevcutOdunc = kutuphaneVeri.ilkOduncKaydi;
    OduncIslemi* sonrakiOdunc;
    while(mevcutOdunc != NULL) {
        sonrakiOdunc = mevcutOdunc->sonrakiOduncIslemi;
        free(mevcutOdunc);
        mevcutOdunc = sonrakiOdunc;
    }
    kutuphaneVeri.ilkOduncKaydi = NULL;

    printf("Tum dinamik bellek alanlari serbest birakildi. Programdan cikiliyor.\n");
    return 0;
}


// Hangi dosyayý, kimin için okuyacaðýný ve her satýrý nasýl iþleyeceðini parametre olarak alýyor.
void genelDosyaOku(KutuphaneVerisi* veri, const char* dosyaAdi, const char* varlikAdi, SatirIsleyiciFonksiyon satirIsle) {
    FILE* fp = fopen(dosyaAdi, "r");
    char satir[512];

    if (!fp) {
        printf("%s dosyasi bulunamadi. Bos %s listesiyle baslaniyor.\n", dosyaAdi, varlikAdi);
        return;
    }

    // Dosyadaki her satýrý okuyup, verilen satirIsle fonksiyonuyla iþliyor.
    while (satirAl(fp, satir, sizeof(satir))) {
        if (strlen(satir) > 0) {
            satirIsle(veri, satir);
        }
    }
    fclose(fp);
    printf("%s %s dosyasindan yuklendi.\n", varlikAdi, dosyaAdi);
}

//Hangi dosyaya, hangi varlýðý, listenin baþýný, her düðümü nasýl yazacaðýný ve sonraki düðümü nasýl alacaðýný parametre olarak alýyor.
void genelDosyaYaz(const char* dosyaAdi, const char* varlikAdi, void* ilkDugum, DugumYazdiriciFonksiyon dugumYazdir, SonrakiDugumAlFonksiyonu sonrakiDugumuAl) {
    FILE* fp = fopen(dosyaAdi, "w");
    void* mevcut = ilkDugum;
    int items_written_total = 0;
    int current_write_failed = 0;

    if (!fp) {
        perror("HATA: Dosya yazmak icin acilamadi");
        fprintf(stderr, " (Dosya: %s)\n", dosyaAdi);
        return;
    }

    // Listenin sonuna kadar git ve her düðümü dugumYazdir fonksiyonuyla dosyaya yazdýrma iþlemi
    while (mevcut != NULL && !current_write_failed) {
        if (!dugumYazdir(fp, mevcut)) {
            current_write_failed = 1;
        } else {
            items_written_total++;
            mevcut = sonrakiDugumuAl(mevcut);
        }
    }

    if (fclose(fp) == EOF) {
        perror("HATA: Dosya kapatilirken hata");
        fprintf(stderr, " (Dosya: %s)\n", dosyaAdi);
    } else {
        if (current_write_failed) {
            printf("UYARI: %s dosyasina (%s) yazma islemi sirasinda bir hata olustugu icin tam olarak kaydedilemedi.\n", varlikAdi, dosyaAdi);
        } else if (items_written_total > 0) {
            printf("%s (%d adet) %s dosyasina basariyla kaydedildi.\n", varlikAdi, items_written_total, dosyaAdi);
        } else if (ilkDugum == NULL) {
            printf("Bilgi: Kaydedilecek %s yoktu (%s listesi bos). %s dosyasi (muhtemelen) bosaltildi.\n", varlikAdi, varlikAdi, dosyaAdi);
        } else {
             printf("UYARI: %s listesi bos degildi ancak hicbir %s %s dosyasina yazilamadi.\n", varlikAdi, varlikAdi, dosyaAdi);
        }
    }
}


// Listenin baþýný, baþlýklarý, her kaydý nasýl göstereceðini, sonraki kaydý nasýl alacaðýný ve bulunamadý mesajýný parametre alýyor.
void genelKayitListele(void* ilkDugum,
                       const char* listeBasligi,
                       const char* sutunBasliklari,
                       TekKayitGosterFonksiyonu tekKayitGoster,
                       SonrakiDugumAlFonksiyonu sonrakiKaydiAl,
                       const char* bulunamadiMesaji,
                       const char* ayiracCizgisi) {
    void* mevcut = ilkDugum;
    printf("\n--- %s ---\n", listeBasligi);
    if (!mevcut) {
        printf("%s\n", bulunamadiMesaji);
        if (ayiracCizgisi && strlen(ayiracCizgisi) > 0) printf("%s\n", ayiracCizgisi);
        return;
    }
    if (sutunBasliklari && strlen(sutunBasliklari) > 0) {
        printf("%s\n", sutunBasliklari);
    }
    if (ayiracCizgisi && strlen(ayiracCizgisi) > 0) printf("%s\n", ayiracCizgisi);

    // Listenin sonuna kadar git ve her kaydý tekKayitGosterile yazdýr.
    while (mevcut != NULL) {
        tekKayitGoster(mevcut);
        mevcut = sonrakiKaydiAl(mevcut);
    }
    if (ayiracCizgisi && strlen(ayiracCizgisi) > 0) printf("%s\n", ayiracCizgisi);
}

int satirAl(FILE* fp, char* tampon, size_t tampon_boyutu) {
    if (tampon_boyutu == 0) return 0;

    if (fgets(tampon, (int)tampon_boyutu, fp) == NULL) {
        if (feof(fp) && tampon_boyutu > 0) {
             tampon[0] = '\0';
        }
        return 0;
    }

    char* newline_char = strchr(tampon, '\n');
    if (newline_char != NULL) {
        *newline_char = '\0';
        if (newline_char > tampon && *(newline_char - 1) == '\r') {
            *(newline_char - 1) = '\0';
        }
    } else {
        if (strlen(tampon) == tampon_boyutu - 1 && fp == stdin) {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
        }
    }

    size_t current_len = strlen(tampon);
    if (current_len > 0 && tampon[current_len-1] == '\r') {
        tampon[current_len-1] = '\0';
    }

    return 1;
}

// stringin baþýndaki ve sonundaki boþluklarý silme iþlemi
void bosluklariSil(char *str) {
    char *end;
    int len;
    char *read_ptr = str;
    char *write_ptr = str;

    if (str == NULL || *str == '\0') {
        return;
    }

    while (isspace((unsigned char)*read_ptr)) {
        read_ptr++;
    }

    if (*read_ptr == '\0') {
        *str = '\0';
        return;
    }

    while ((*write_ptr++ = *read_ptr++));

    len = strlen(str);
    if (len == 0) return;
    end = str + len - 1;
    while (end >= str && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';
}

//tarihi gün, ay, yýl olarak ayýrýyor.
void tarihiParcala(const char* tarih_str, int* gun, int* ay, int* yil) {
    if (sscanf(tarih_str, "%d.%d.%d", gun, ay, yil) != 3) {
        *gun = 0; *ay = 0; *yil = 0;
    }
}

// Ödünç süresi kontrolü için Ýki tarih arasýndaki gün farkýný hesaplýyor.
int gunFarkiniHesapla(const char* tarih1_str, const char* tarih2_str) {
    int d1, a1, y1, d2, a2, y2;
    struct tm zamanbilgisi1 = {0}, zamanbilgisi2 = {0};
    time_t t1, t2;
    double fark_saniye;

    tarihiParcala(tarih1_str, &d1, &a1, &y1);
    tarihiParcala(tarih2_str, &d2, &a2, &y2);

    if (y1 == 0 || y2 == 0) {
        return -9999;
    }

    zamanbilgisi1.tm_year = y1 - 1900;
    zamanbilgisi1.tm_mon = a1 - 1;
    zamanbilgisi1.tm_mday = d1;
    zamanbilgisi1.tm_isdst = -1;

    zamanbilgisi2.tm_year = y2 - 1900;
    zamanbilgisi2.tm_mon = a2 - 1;
    zamanbilgisi2.tm_mday = d2;
    zamanbilgisi2.tm_isdst = -1;

    t1 = mktime(&zamanbilgisi1);
    t2 = mktime(&zamanbilgisi2);

    if (t1 == (time_t)-1 || t2 == (time_t)-1) {
        return -9999;
    }

    fark_saniye = difftime(t2, t1);
    return (int)(fark_saniye / (60 * 60 * 24));
}

// Bugünün tarihini formatýnda alýyor.
void bugununTarihiniAl(char* tarih_tamponu) {
    time_t t = time(NULL);
    struct tm* tm_info_ptr;

    tm_info_ptr = localtime(&t);

    if (tm_info_ptr != NULL) {
        sprintf(tarih_tamponu, "%02d.%02d.%d", tm_info_ptr->tm_mday, tm_info_ptr->tm_mon + 1, tm_info_ptr->tm_year + 1900);
    } else {
        strcpy(tarih_tamponu, "00.00.0000");
        fprintf(stderr, "Hata: Guncel tarih alinamadi.\n");
    }
}

void yazarSatiriniIsle(KutuphaneVerisi* veri, char* satir) {
    int id;
    char ad_buffer[MAX_AD_UZUNLUK];
    char soyad_buffer[MAX_SOYAD_UZUNLUK];
    YazarBilgisi* yzr;

    if (sscanf(satir, "%d ,%49[^,] ,%49s", &id, ad_buffer, soyad_buffer) == 3 ||
        sscanf(satir, "%d,%49[^,],%49s", &id, ad_buffer, soyad_buffer) == 3) {
        bosluklariSil(ad_buffer);
        bosluklariSil(soyad_buffer);
        yzr = yeniYazarDugumuOlustur(id, ad_buffer, soyad_buffer);
        if (yzr) {
            yazariListeyeEkle(veri, yzr);
        }
    } else if (strlen(satir) > 0) {
        fprintf(stderr, "UYARI [yazarSatiriniIsle]: Yazar satiri '%s' okunamadi veya format hatali.\n", satir);
    }
}

// Bir yazar düðümünü dosyaya yazdýrýyor.
int yazarDugumunuYazdir(FILE* fp, void* dugum) {
    YazarBilgisi* mevcut = (YazarBilgisi*)dugum;
    if (fprintf(fp, "%d,%s,%s\n", mevcut->yazarNo, mevcut->adi, mevcut->soyadi) < 0) {
        fprintf(stderr, "HATA: Yazar verisi (ID: %d) dosyaya yazilirken bir sorun olustu.\n", mevcut->yazarNo);
        return 0;
    }
    return 1;
}

// Yazar listesinde bir sonraki düðümü döndürüyor.
void* sonrakiYazarDugumunuAl(void* dugum) {
    if (!dugum) return NULL;
    return ((YazarBilgisi*)dugum)->sonrakiYazar;
}

// genelKayitListele ile bir yazarýn bilgilerini ekrana yazdýrýyor.
void tekYazarGoster(void* kayit) {
    YazarBilgisi* y = (YazarBilgisi*)kayit;
    printf("%-5d %-20s %-20s\n", y->yazarNo, y->adi, y->soyadi);
}

// Öðrenci CSV dosyasýndaki bir satýrý iþleyip öðrenci listesine ekliyor.
void ogrenciSatiriniIsle(KutuphaneVerisi* veri, char* satir) {
    char id_buffer[OGRENCI_NO_UZUNLUK];
    char ad_buffer[MAX_AD_UZUNLUK];
    char soyad_buffer[MAX_SOYAD_UZUNLUK];
    int puan_buffer;
    OgrenciBilgisi* ogr;

    if (sscanf(satir, "%8[^,] ,%49[^,] ,%49[^,] ,%d", id_buffer, ad_buffer, soyad_buffer, &puan_buffer) == 4 ||
        sscanf(satir, "%8[^,],%49[^,],%49[^,],%d", id_buffer, ad_buffer, soyad_buffer, &puan_buffer) == 4) {
        bosluklariSil(id_buffer);
        bosluklariSil(ad_buffer);
        bosluklariSil(soyad_buffer);
        ogr = ogrenciOlustur(id_buffer, ad_buffer, soyad_buffer, puan_buffer);
        if (ogr) {
            if (!ogrenciyiListeyeEkle(veri, ogr)) {
            }
        }
    } else if (strlen(satir) > 0) {
        fprintf(stderr, "UYARI [ogrenciSatiriniIsle]: Ogrenci satiri '%s' okunamadi veya format hatali.\n", satir);
    }
}

// Bir öðrenci düðümünü dosyaya yazdýrýyor.
int ogrenciDugumunuYazdir(FILE* fp, void* dugum) {
    OgrenciBilgisi* mevcut = (OgrenciBilgisi*)dugum;
    if (fprintf(fp, "%s,%s,%s,%d\n", mevcut->ogrNo, mevcut->adi, mevcut->soyadi, mevcut->puan) < 0) {
        fprintf(stderr, "HATA: Ogrenci verisi (No: %s) dosyaya yazilirken bir sorun olustu.\n", mevcut->ogrNo);
        return 0;
    }
    return 1;
}

// Öðrenci listesinde bir sonraki düðümü döndürüyor.
void* sonrakiOgrenciDugumunuAl(void* dugum) {
    if (!dugum) return NULL;
    return ((OgrenciBilgisi*)dugum)->sonrakiOgrenci;
}

// Tek bir öðrencinin bilgilerini ekrana yazdýrýyor.
void tekOgrenciGoster(void* kayit) {
    OgrenciBilgisi* o = (OgrenciBilgisi*)kayit;
    printf("%-10s %-20s %-20s %-6d\n", o->ogrNo, o->adi, o->soyadi, o->puan);
}

// Kitaplar CSV dosyasýndaki bir satýrý iþleyip kitap listesine ekliyor.
void kitapAnaBilgiSatiriniIsle(KutuphaneVerisi* veri, char* satir) {
    char isbn_buffer[ISBN_UZUNLUK];
    char ad_kitap_buffer[100];
    int adet_dosyadan_kullanilmiyor;
    KitapBilgisi* kitap_ptr;

    // Kitaplar.csv formatý: kitapAdý, ISBN, adet (virgülden sonra bir boþluk olduðu varsayýlýyor)
    int sscanf_result = sscanf(satir, "%99[^,], %13[^,], %d", ad_kitap_buffer, isbn_buffer, &adet_dosyadan_kullanilmiyor);

    if (sscanf_result == 3) {
        bosluklariSil(ad_kitap_buffer);
        bosluklariSil(isbn_buffer);

        if (strlen(isbn_buffer) != (ISBN_UZUNLUK - 1)) {
            fprintf(stderr, "UYARI [kitapAnaBilgiSatiriniIsle]: Gecersiz ISBN uzunlugu ('%s' - %zu karakter) '%s' satirinda. Kayit atlaniyor.\n",
                    isbn_buffer, strlen(isbn_buffer), satir);
            return;
        }

        kitap_ptr = kitapOlustur(isbn_buffer, ad_kitap_buffer, 0);
        if(kitap_ptr){
            kitabiListeyeEkle(veri, kitap_ptr);
        } else {
            fprintf(stderr, "HATA [kitapAnaBilgiSatiriniIsle]: Kitap olusturulamadi (ISBN: %s) '%s' satirindan.\n", isbn_buffer, satir);
        }
    } else if (strlen(satir) > 0) {
         fprintf(stderr, "UYARI [kitapAnaBilgiSatiriniIsle]: Kitap ana bilgi satiri '%s' okunamadi veya format hatali (sscanf %d dondu).\n",
                 satir, sscanf_result);
    }
}

// Bir kitabýn ana bilgilerini dosyaya yazdýrýyor.
int kitapAnaBilgiDugumunuYazdir(FILE* fp, void* dugum) {
    KitapBilgisi* mevcut_kitap = (KitapBilgisi*)dugum;
    if (fprintf(fp, "%s,%s,%d\n", mevcut_kitap->kitapAdi, mevcut_kitap->isbnNo, mevcut_kitap->kopyaSayisi) < 0) {
         fprintf(stderr, "HATA: Kitap ana verisi (ISBN: %s) dosyaya yazilirken sorun olustu.\n", mevcut_kitap->isbnNo);
         return 0;
    }
    return 1;
}

// Kitap listesinde bir sonraki düðümü döndürüyor.
void* sonrakiKitapAnaBilgiDugumunuAl(void* dugum) {
    if (!dugum) return NULL;
    return ((KitapBilgisi*)dugum)->sonrakiKitap;
}

// Ödünç kayýtlarý CSV dosyasýndaki bir satýrý iþleyip listeye ekliyor.
void oduncKaydiSatiriniIsle(KutuphaneVerisi* veri, char* satir) {
    char etiket_buffer[KITAP_ETIKET_UZUNLUK];
    char ogrID_buffer[OGRENCI_NO_UZUNLUK];
    int tur_buffer;
    char tarih_str_buffer[TARIH_UZUNLUK];
    OduncIslemi* kyt;

    if (sscanf(satir, "%29[^,] ,%8[^,] ,%d ,%10s", etiket_buffer, ogrID_buffer, &tur_buffer, tarih_str_buffer) == 4 ||
        sscanf(satir, "%29[^,],%8[^,],%d,%10s", etiket_buffer, ogrID_buffer, &tur_buffer, tarih_str_buffer) == 4) {
        bosluklariSil(etiket_buffer);
        bosluklariSil(ogrID_buffer);
        bosluklariSil(tarih_str_buffer);
        kyt = yeniOduncKaydiOlustur(etiket_buffer, ogrID_buffer, tur_buffer, tarih_str_buffer);
        if (kyt) {
             oduncKaydiniListeyeEkle(veri, kyt);
        }
    } else if (strlen(satir) > 0) {
        fprintf(stderr, "UYARI [oduncKaydiSatiriniIsle]: Odunc kaydi satiri '%s' okunamadi veya format hatali.\n", satir);
    }
}

// Bir ödünç kaydý düðümünü dosyaya yazdýrýyor.
int oduncKaydiDugumunuYazdir(FILE* fp, void* dugum) {
    OduncIslemi* mevcut = (OduncIslemi*)dugum;
    if (fprintf(fp, "%s,%s,%d,%s\n", mevcut->oduncKitapEtiketi, mevcut->oduncAlanOgrNo, mevcut->islemTipi, mevcut->islemTarihi) < 0) {
        fprintf(stderr, "HATA: Odunc kaydi (%s-%s) dosyaya yazilirken sorun olustu.\n",
                mevcut->oduncKitapEtiketi, mevcut->oduncAlanOgrNo);
        return 0;
    }
    return 1;
}

// Ödünç kaydý listesinde bir sonraki düðümü döndürüyor.
void* sonrakiOduncKaydiDugumunuAl(void* dugum) {
    if (!dugum) return NULL;
    return ((OduncIslemi*)dugum)->sonrakiOduncIslemi;
}

// Yazarlarý dosyadan okumak için genelDosyaOku'yu çaðýrýyor.
// Sonra da bir sonraki yazar numarasýný güncelliyor.
void yazarlariDosyadanAl(KutuphaneVerisi* veri) {
    genelDosyaOku(veri, YAZARLAR_DOSYASI, "Yazarlar", yazarSatiriniIsle);
    int bulunan_max_id = 0;
    YazarBilgisi* temp_yazar = veri->ilkYazar;
    while(temp_yazar != NULL) {
        if (temp_yazar->yazarNo > bulunan_max_id) {
            bulunan_max_id = temp_yazar->yazarNo;
        }
        temp_yazar = temp_yazar->sonrakiYazar;
    }
    veri->siradakiYazarNo = bulunan_max_id + 1;
}

// Yazarlarý dosyaya yazmak için genelDosyaYaz'ý çaðýrýyor.
void yazarlariDosyayaYaz(KutuphaneVerisi* veri) {
    genelDosyaYaz(YAZARLAR_DOSYASI, "Yazarlar", veri->ilkYazar, yazarDugumunuYazdir, sonrakiYazarDugumunuAl);
}

// Öðrencileri dosyadan okumak için genelDosyaOku'yu çaðýrýyor.
void ogrencileriDosyadanAl(KutuphaneVerisi* veri) {
    genelDosyaOku(veri, OGRENCILER_DOSYASI, "Ogrenciler", ogrenciSatiriniIsle);
}

// Öðrencileri dosyaya yazmak için genelDosyaYaz'ý çaðýrýyor.
void ogrencileriDosyayaYaz(KutuphaneVerisi* veri) {
    genelDosyaYaz(OGRENCILER_DOSYASI, "Ogrenciler", veri->ilkOgrenci, ogrenciDugumunuYazdir, sonrakiOgrenciDugumunuAl);
}

// Kitaplarýn ana bilgilerini dosyadan okumak için genelDosyaOku'yu çaðýrýyor.
void kitapAnaBilgileriniDosyadanAl(KutuphaneVerisi* veri) {
    genelDosyaOku(veri, KITAPLAR_DOSYASI, "Ana Kitaplar", kitapAnaBilgiSatiriniIsle);
}

// Kitaplarýn ana bilgilerini dosyaya yazmak için genelDosyaYaz'ý çaðýrýyor.
void kitapAnaBilgileriniDosyayaYaz(KutuphaneVerisi* veri) {
    genelDosyaYaz(KITAPLAR_DOSYASI, "Kitaplar (Ana Bilgi)", veri->ilkKitap, kitapAnaBilgiDugumunuYazdir, sonrakiKitapAnaBilgiDugumunuAl);
}

// Ödünç kayýtlarýný dosyadan okumak için genelDosyaOku'yu çaðýrýyor.
void oduncKayitlariniDosyadanAl(KutuphaneVerisi* veri) {
    genelDosyaOku(veri, ODUNC_KAYITLARI_DOSYASI, "Odunc Kayitlari", oduncKaydiSatiriniIsle);
}

// Ödünç kayýtlarýný dosyaya yazmak için genelDosyaYaz'ý çaðýrýyor.
void oduncKayitlariniDosyayaYaz(KutuphaneVerisi* veri) {
    genelDosyaYaz(ODUNC_KAYITLARI_DOSYASI, "Odunc Kayitlari", veri->ilkOduncKaydi, oduncKaydiDugumunuYazdir, sonrakiOduncKaydiDugumunuAl);
}

//Kitap kopyalarýný dosyadan okuyup ilgili ana kitapla iliþkilendiriyor.
// Etiketten ISBN'i ayýrýp ana kitabý buluyor, sonra kopyayý o kitaba ekliyor.
void kitapKopyalariniYukleVeIliskilendir(KutuphaneVerisi* veri) {
    FILE* fp_kopyalar = fopen(KITAP_KOPYALARI_DOSYASI, "r");
    char satir[256];
    char etiket_numarasi_buffer[KITAP_ETIKET_UZUNLUK];
    char durum_buffer[OGRENCI_NO_UZUNLUK];
    char orijinal_isbn[ISBN_UZUNLUK];
    char* alt_cizgi_ptr;
    KitapBilgisi* ana_kitap;
    KitapKopyasi* kopya_ptr;
    size_t isbn_len;

    if (!fp_kopyalar) {
        printf("%s bulunamadi. Kopyalar yuklenemedi.\n", KITAP_KOPYALARI_DOSYASI);
        return;
    }

    printf("Kitap kopyalari %s dosyasindan yukleniyor...\n", KITAP_KOPYALARI_DOSYASI);
    while (satirAl(fp_kopyalar, satir, sizeof(satir))) {
        if (sscanf(satir, "%29[^,] ,%8s", etiket_numarasi_buffer, durum_buffer) == 2 ||
            sscanf(satir, "%29[^,],%8s", etiket_numarasi_buffer, durum_buffer) == 2) {
            bosluklariSil(etiket_numarasi_buffer);
            bosluklariSil(durum_buffer);

            alt_cizgi_ptr = strrchr(etiket_numarasi_buffer, '_');
            if (alt_cizgi_ptr) {
                isbn_len = alt_cizgi_ptr - etiket_numarasi_buffer;
                if (isbn_len == (ISBN_UZUNLUK - 1)) {
                    strncpy(orijinal_isbn, etiket_numarasi_buffer, isbn_len);
                    orijinal_isbn[isbn_len] = '\0';

                    ana_kitap = kitapBulIsbnIle(veri, orijinal_isbn);
                    if (ana_kitap) {
                        kopya_ptr = (KitapKopyasi*)malloc(sizeof(KitapKopyasi));
                        if (!kopya_ptr) {
                            perror("KitapKopyasi icin bellek ayrilamadi (yukleme)");
                        } else {
                            strncpy(kopya_ptr->kopyaEtiketi, etiket_numarasi_buffer, KITAP_ETIKET_UZUNLUK -1);
                            kopya_ptr->kopyaEtiketi[KITAP_ETIKET_UZUNLUK-1] = '\0';

                            strncpy(kopya_ptr->durumu, durum_buffer, OGRENCI_NO_UZUNLUK-1);
                            kopya_ptr->durumu[OGRENCI_NO_UZUNLUK-1] = '\0';

                            kopya_ptr->sonrakiKopya = ana_kitap->kitapKopyalari;
                            ana_kitap->kitapKopyalari = kopya_ptr;
                            ana_kitap->kopyaSayisi++;
                        }
                    } else {
                         fprintf(stderr, "UYARI [kitapKopyalariniYukle]: Etiket '%s' icin ana kitap (ISBN: %s) bulunamadi. Kopya atlaniyor.\n", etiket_numarasi_buffer, orijinal_isbn);
                    }
                } else {
                     fprintf(stderr, "UYARI [kitapKopyalariniYukle]: Etiket '%s' icindeki ISBN kismi gecersiz uzunlukta (%zu olmaliydi %zu). Kopya atlaniyor.\n", etiket_numarasi_buffer, (size_t)(ISBN_UZUNLUK - 1), isbn_len);
                }
            } else {
                 fprintf(stderr, "UYARI [kitapKopyalariniYukle]: Etiket '%s' formatinda '_' karakteri bulunamadi. Kopya atlaniyor.\n", etiket_numarasi_buffer);
            }
        } else if (strlen(satir) > 0) {
            fprintf(stderr, "UYARI [kitapKopyalariniYukle]: Kopya satiri '%s' okunamadi veya format hatali.\n", satir);
        }
    }
    fclose(fp_kopyalar);
    printf("Kitap kopyalari %s dosyasindan yuklendi ve iliskilendirildi.\n", KITAP_KOPYALARI_DOSYASI);
}

// Tüm kitaplarýn kopyalarýný dosyaya yazýyor. Her kitabýn kopyalarýný tek tek dolaþýyor.
void kitapKopyalariniDosyayaYaz(KutuphaneVerisi* veri) {
    FILE* fp_kopyalar = fopen(KITAP_KOPYALARI_DOSYASI, "w");
    KitapBilgisi* mevcut_kitap_iter = veri->ilkKitap;
    KitapKopyasi* mevcut_kopya_iter;
    int kopyalar_yazildi_mi = 0;
    int toplam_kopya_kaydedildi = 0;
    int yazma_hatasi_olustu = 0;

    if (!fp_kopyalar) {
        perror("HATA: Kitap kopyalari dosyasi yazmak icin acilirken hata");
        return;
    }

    while (mevcut_kitap_iter != NULL && !yazma_hatasi_olustu) {
        mevcut_kopya_iter = mevcut_kitap_iter->kitapKopyalari;
        while (mevcut_kopya_iter != NULL && !yazma_hatasi_olustu) {
            if (fprintf(fp_kopyalar, "%s,%s\n", mevcut_kopya_iter->kopyaEtiketi, mevcut_kopya_iter->durumu) < 0) {
                fprintf(stderr, "HATA: Kitap kopya verisi (%s) dosyaya yazilirken sorun olustu.\n", mevcut_kopya_iter->kopyaEtiketi);
                yazma_hatasi_olustu = 1;
            } else {
                kopyalar_yazildi_mi = 1;
                toplam_kopya_kaydedildi++;
                mevcut_kopya_iter = mevcut_kopya_iter->sonrakiKopya;
            }
        }
        if (!yazma_hatasi_olustu) {
             mevcut_kitap_iter = mevcut_kitap_iter->sonrakiKitap;
        }
    }

    if (yazma_hatasi_olustu) {
        fclose(fp_kopyalar);
        printf("UYARI: Kitap kopyalari %s dosyasina yazilirken hata olustu, islem durduruldu.\n", KITAP_KOPYALARI_DOSYASI);
        return;
    }


    if (fclose(fp_kopyalar) == EOF) {
         perror("HATA: Kitap kopyalari dosyasi kapatilirken hata");
    } else {
        if (kopyalar_yazildi_mi) {
            printf("Kitap kopyalari (%d adet) %s dosyasina basariyla kaydedildi.\n", toplam_kopya_kaydedildi, KITAP_KOPYALARI_DOSYASI);
        } else if (veri->ilkKitap == NULL) {
             printf("Bilgi: Kaydedilecek kitap olmadigi icin kopya da yoktu. %s dosyasi (muhtemelen) bosaltildi.\n", KITAP_KOPYALARI_DOSYASI);
        } else {
            int toplam_kopya_sayisi_sistemde = 0;
            KitapBilgisi* k_temp = veri->ilkKitap;
            while(k_temp) { toplam_kopya_sayisi_sistemde += k_temp->kopyaSayisi; k_temp = k_temp->sonrakiKitap; }

            if (toplam_kopya_sayisi_sistemde == 0) {
                 printf("Bilgi: Kaydedilecek kitap kopyasi yoktu (tum kitaplarin kopya sayisi 0). %s dosyasi (muhtemelen) bosaltildi.\n", KITAP_KOPYALARI_DOSYASI);
            } else {
                 printf("UYARI: Kitap kopyalari listesi bos degildi (%d adet sistemde) ancak hicbir kopya %s dosyasina yazilamadi.\n", toplam_kopya_sayisi_sistemde, KITAP_KOPYALARI_DOSYASI);
            }
        }
    }
}

// Kitap-Yazar iliþkilerini okuyor.
// Önce dosyadaki kayýt sayýsýný buluyor sonra yer ayýrýp okuyor.
void kitapYazarIliskileriniDosyadanAl(KutuphaneVerisi* veri) {
    FILE* fp = fopen(KITAP_YAZAR_DOSYASI, "r");
    char satir[256];
    int N = 0;
    char isbn_buffer[ISBN_UZUNLUK];
    int yazar_id_buffer;

    if (!fp) {
        printf("%s dosyasi bulunamadi. Kitap-Yazar iliskileri yuklenemedi.\n", KITAP_YAZAR_DOSYASI);
        veri->kitapYazarIliskileri = NULL;
        veri->kitapYazarAdedi = 0;
        return;
    }

    // Dosyayýoku, geçerli satýr sayýsýný
    while (satirAl(fp, satir, sizeof(satir))) {
        if (sscanf(satir, "%13[^,] ,%d", isbn_buffer, &yazar_id_buffer) == 2 ||
            sscanf(satir, "%13[^,],%d", isbn_buffer, &yazar_id_buffer) == 2) {
            N++;
        }
    }
    rewind(fp);

    if (N == 0) {
        fclose(fp);
        printf("%s dosyasi bos veya gecerli kayit icermiyor. Kitap-Yazar iliskisi yok.\n", KITAP_YAZAR_DOSYASI);
        veri->kitapYazarIliskileri = NULL;
        veri->kitapYazarAdedi = 0;
        return;
    }

    // N kadar yer ayýr.
    veri->kitapYazarIliskileri = (KitapYazarIliskisi*)malloc(N * sizeof(KitapYazarIliskisi));
    if (!veri->kitapYazarIliskileri) {
        perror("Kitap-yazar iliskileri icin bellek ayrilamadi (yukleme)");
        fclose(fp);
        veri->kitapYazarAdedi = 0;
        return;
    }

    veri->kitapYazarAdedi = 0;
    // Dosyayý tekrar oku ve bu sefer diziye yükle.
    while (satirAl(fp, satir, sizeof(satir)) && veri->kitapYazarAdedi < N) {
        if (sscanf(satir, "%13[^,] ,%d", isbn_buffer, &yazar_id_buffer) == 2 ||
            sscanf(satir, "%13[^,],%d", isbn_buffer, &yazar_id_buffer) == 2) {
            bosluklariSil(isbn_buffer);
            if (kitapBulIsbnIle(veri, isbn_buffer) != NULL && (yazarBulNoIle(veri, yazar_id_buffer) != NULL || yazar_id_buffer == -1) ) {
                 strncpy(veri->kitapYazarIliskileri[veri->kitapYazarAdedi].kitabinIsbn, isbn_buffer, ISBN_UZUNLUK - 1);
                 veri->kitapYazarIliskileri[veri->kitapYazarAdedi].kitabinIsbn[ISBN_UZUNLUK - 1] = '\0';
                 veri->kitapYazarIliskileri[veri->kitapYazarAdedi].yazarID = yazar_id_buffer;
                 veri->kitapYazarAdedi++;
            } else {
                fprintf(stderr, "UYARI [kitap_yazar_yukle]: Satir '%s' gecersiz ISBN veya Yazar ID iceriyor. Atlaniyor.\n", satir);
            }
        } else if (strlen(satir) > 0) {
            fprintf(stderr, "UYARI [kitap_yazar_yukle]: Satir '%s' okunamadi veya format hatali.\n", satir);
        }
    }
    fclose(fp);

    // Eðer N kadar yer ayrýldý ama daha az geçerli kayýt yüklendiyse, diziyi küçültür.
    if (veri->kitapYazarAdedi < N && veri->kitapYazarAdedi > 0) {
        KitapYazarIliskisi* temp_dizi = (KitapYazarIliskisi*)realloc(veri->kitapYazarIliskileri, veri->kitapYazarAdedi * sizeof(KitapYazarIliskisi));
        if (temp_dizi != NULL) {
            veri->kitapYazarIliskileri = temp_dizi;
        }
    } else if (veri->kitapYazarAdedi == 0 && veri->kitapYazarIliskileri != NULL) { // Hiç geçerli kayýt yüklenemediyse ve bellek ayrýldýysa, serbest býrak.
        free(veri->kitapYazarIliskileri);
        veri->kitapYazarIliskileri = NULL;
    }

    printf("%d Kitap-Yazar iliskisi %s dosyasindan yuklendi.\n", veri->kitapYazarAdedi, KITAP_YAZAR_DOSYASI);
}

// Kitap-Yazar iliþkilerini dosyaya yazýyor.
void kitapYazarIliskileriniDosyayaYaz(KutuphaneVerisi* veri) {
    FILE* fp = fopen(KITAP_YAZAR_DOSYASI, "w");
    int i;
    int yazilan_adet = 0;
    int yazma_hatasi = 0;

    if (!fp) {
        perror("HATA: Kitap-yazar iliskileri dosyasi yazmak icin acilirken hata");
        return;
    }
    for (i = 0; i < veri->kitapYazarAdedi && !yazma_hatasi; i++) {
        if (fprintf(fp, "%s,%d\n", veri->kitapYazarIliskileri[i].kitabinIsbn, veri->kitapYazarIliskileri[i].yazarID) < 0) {
            fprintf(stderr, "HATA: Kitap-Yazar iliskisi (%s-%d) dosyaya yazilirken sorun olustu.\n",
                    veri->kitapYazarIliskileri[i].kitabinIsbn, veri->kitapYazarIliskileri[i].yazarID);
            yazma_hatasi = 1;
        } else {
            yazilan_adet++;
        }
    }
    if (fclose(fp) == EOF) {
        perror("HATA: Kitap-Yazar iliskileri dosyasi kapatilirken hata");
    } else {
        if (yazma_hatasi) {
             printf("UYARI: Kitap-Yazar iliskilerinin tamami (%d/%d) %s dosyasina kaydedilemedi.\n", yazilan_adet, veri->kitapYazarAdedi, KITAP_YAZAR_DOSYASI);
        } else if (yazilan_adet == veri->kitapYazarAdedi && veri->kitapYazarAdedi > 0) {
            printf("Kitap-Yazar iliskileri (%d adet) %s dosyasina kaydedildi.\n", yazilan_adet, KITAP_YAZAR_DOSYASI);
        } else if (veri->kitapYazarAdedi == 0) {
             printf("Bilgi: Kaydedilecek kitap-yazar iliskisi yoktu. %s dosyasi (muhtemelen) bosaltildi.\n", KITAP_YAZAR_DOSYASI);
        }
    }
}

// Bütün yazarlarý listelemek için genelKayitListele'yi çaðýrýyor.
void butunYazarlariGoster(KutuphaneVerisi* veri) {
    genelKayitListele(veri->ilkYazar,
                      "Tum Yazarlarin Listesi",
                      "ID    Ad                   Soyad               ",
                      tekYazarGoster,
                      sonrakiYazarDugumunuAl,
                      "Sistemde yazar bulunmuyor.",
                      "----------------------------------------------");
}

// Bütün öðrencileri listelemek için genelKayitListele'yi çaðýrýyor.
void butunOgrencileriGoster(KutuphaneVerisi* veri) {
    genelKayitListele(veri->ilkOgrenci,
                      "Tum Ogrencilerin Listesi",
                      "Ogr. No    Ad                   Soyad                Puan  ",
                      tekOgrenciGoster,
                      sonrakiOgrenciDugumunuAl,
                      "Sistemde ogrenci bulunmuyor.",
                      "---------------------------------------------------------");
}

// Yeni bir yazar düðümü oluþturup bilgilerini dolduruyor.
YazarBilgisi* yeniYazarDugumuOlustur(int id, const char* ad_param, const char* soyad_param) {
    YazarBilgisi* yeniYazar = (YazarBilgisi*)malloc(sizeof(YazarBilgisi));
    if (!yeniYazar) { // Bellek ayrýlamazsa NULL dönüyor.
        perror("Yeni yazar icin bellek ayrilamadi");
        return NULL;
    }
    yeniYazar->yazarNo = id;
    strncpy(yeniYazar->adi, ad_param, MAX_AD_UZUNLUK -1);
    yeniYazar->adi[MAX_AD_UZUNLUK-1] = '\0';
    strncpy(yeniYazar->soyadi, soyad_param, MAX_SOYAD_UZUNLUK -1);
    yeniYazar->soyadi[MAX_SOYAD_UZUNLUK-1] = '\0';
    yeniYazar->sonrakiYazar = NULL;
    return yeniYazar;
}

// Yeni yazarý ID'ye göre sýralý bir þekilde listeye ekliyor.
void yazariListeyeEkle(KutuphaneVerisi* veri, YazarBilgisi* yeniYazar_param) {
    YazarBilgisi* mevcut;
    if (!yeniYazar_param) return;

    // Eðer liste boþsa veya yeni yazarýn ID'si ilk yazarýn ID'sinden küçük veya eþitse baþa ekle.
    if (!veri->ilkYazar || veri->ilkYazar->yazarNo >= yeniYazar_param->yazarNo) {
        if (veri->ilkYazar && veri->ilkYazar->yazarNo == yeniYazar_param->yazarNo) {
            printf("Hata: Yazar ID %d zaten mevcut. Eklenemiyor.\n", yeniYazar_param->yazarNo);
            free(yeniYazar_param);
            return;
        }
        yeniYazar_param->sonrakiYazar = veri->ilkYazar;
        veri->ilkYazar = yeniYazar_param;
        return;
    }
    mevcut = veri->ilkYazar;
    while (mevcut->sonrakiYazar != NULL && mevcut->sonrakiYazar->yazarNo < yeniYazar_param->yazarNo) {
        mevcut = mevcut->sonrakiYazar;
    }
    if (mevcut->sonrakiYazar && mevcut->sonrakiYazar->yazarNo == yeniYazar_param->yazarNo) {
        printf("Hata: Yazar ID %d zaten mevcut (araya ekleme). Eklenemiyor.\n", yeniYazar_param->yazarNo);
        free(yeniYazar_param);
        return;
    }
    yeniYazar_param->sonrakiYazar = mevcut->sonrakiYazar;
    mevcut->sonrakiYazar = yeniYazar_param;
}

// Kullanýcýdan yazar bilgilerini alýp ekleme iþlemini yapýyor.
void yazarEkleArayuz(KutuphaneVerisi* veri) {
    char ad_buffer[MAX_AD_UZUNLUK];
    char soyad_buffer[MAX_SOYAD_UZUNLUK];
    YazarBilgisi* yzr;

    printf("Yazar adini girin: > ");
    satirAl(stdin, ad_buffer, sizeof(ad_buffer));
    printf("Yazar soyadini girin: > ");
    satirAl(stdin, soyad_buffer, sizeof(soyad_buffer));

    bosluklariSil(ad_buffer);
    bosluklariSil(soyad_buffer);

    if (strlen(ad_buffer) == 0 || strlen(soyad_buffer) == 0) {
        printf("Yazar adi ve soyadi bos olamaz.\n");
        return;
    }

    yzr = yeniYazarDugumuOlustur(veri->siradakiYazarNo, ad_buffer, soyad_buffer);
    if (yzr) {
        yazariListeyeEkle(veri, yzr);
        if (yazarBulNoIle(veri, yzr->yazarNo) == yzr) {
            veri->siradakiYazarNo++;
            yazarlariDosyayaYaz(veri);
            printf("Yazar %s %s (ID: %d) basariyla eklendi.\n", ad_buffer, soyad_buffer, yzr->yazarNo);
        }
    }
}

// Verilen ID'ye sahip yazarý listede bulup döndürüyor. Bulamazsa NULL.
YazarBilgisi* yazarBulNoIle(KutuphaneVerisi* veri, int id_param) {
    YazarBilgisi* mevcut = veri->ilkYazar;
    while (mevcut != NULL) {
        if (mevcut->yazarNo == id_param) {
            return mevcut;
        }
        mevcut = mevcut->sonrakiYazar;
    }
    return NULL;
}

// Kullanýcýdan ID alýp yazarý siliyor. Silmeden önce onay istiyor.
// Yazar silinince, o yazara ait kitap-yazar iliþkilerindeki yazar ID'sini -1 yapýyor.
void yazarSilArayuz(KutuphaneVerisi* veri) {
    int silinecek_id;
    YazarBilgisi* onceki = NULL;
    YazarBilgisi* mevcut = veri->ilkYazar;
    char onay[3];
    int bulundu = 0;

    printf("Silinecek Yazar ID'sini girin: > ");
    if (scanf("%d", &silinecek_id) != 1) {
        while(getchar()!='\n');
        printf("ID icin gecersiz giris.\n");
        return;
    }
    while(getchar()!='\n');

    // Silinecek yazarý bul.
    mevcut = veri->ilkYazar;
    while (mevcut != NULL && !bulundu) {
        if (mevcut->yazarNo == silinecek_id) {
            bulundu = 1;
        } else {
            onceki = mevcut;
            mevcut = mevcut->sonrakiYazar;
        }
    }

    if (!bulundu) {
        printf("ID %d olan yazar bulunamadi.\n", silinecek_id);
        return;
    }


    printf("Yazar %s %s (ID: %d) silinecek. Bu yazarla iliskili tum kitap-yazar kayitlari da guncellenecektir (Yazar ID'si -1 olacak). Emin misiniz? (E/H): > ", mevcut->adi, mevcut->soyadi, mevcut->yazarNo);
    satirAl(stdin, onay, sizeof(onay));
    bosluklariSil(onay);
    if (strcmp(onay, "E") != 0 && strcmp(onay, "e") != 0) { // 'E' veya 'e' deðilse iptal.
        printf("Yazar silme islemi iptal edildi.\n");
        return;
    }

    // Yazarý listeden sil.
    if (onceki == NULL) { // Listenin baþýndaysa.
        veri->ilkYazar = mevcut->sonrakiYazar;
    } else { // Ortada veya sondaysa.
        onceki->sonrakiYazar = mevcut->sonrakiYazar;
    }

    yazar_silindiginde_kitap_yazari_guncelle(veri, silinecek_id); // Kitap-yazar iliþkilerini güncelle.
    free(mevcut); // Yazarý serbest býrak.

    yazarlariDosyayaYaz(veri); // Deðiþiklikleri dosyaya kaydet.
    kitapYazarIliskileriniDosyayaYaz(veri);
    printf("Yazar (ID: %d) silindi ve ilgili kayitlar guncellendi.\n", silinecek_id);
}

// Bir yazar silindiðinde, o yazara ait tüm kitap-yazar iliþkilerindeki yazar ID'sini -1 (silinmiþ yazar) olarak güncelliyor.
void yazar_silindiginde_kitap_yazari_guncelle(KutuphaneVerisi* veri, int silinenYazarID) {
    int i;
    for (i = 0; i < veri->kitapYazarAdedi; i++) { // Tüm kitap-yazar iliþkilerini dolaþ.
        if (veri->kitapYazarIliskileri[i].yazarID == silinenYazarID) { // Silinen yazarla eþleþiyorsa.
            veri->kitapYazarIliskileri[i].yazarID = -1; // Yazar ID'sini -1 yap.
        }
    }
}

// Kullanýcýdan ID alýp yazar bilgilerini düzenliyor. Deðiþtirmek istemediði alanlar için Enter'a basabilir.
void yazarDuzenleArayuz(KutuphaneVerisi* veri) {
    int duzenlenecek_id;
    YazarBilgisi* yazar_ptr;
    char yeni_ad_buffer[MAX_AD_UZUNLUK];
    char yeni_soyad_buffer[MAX_SOYAD_UZUNLUK];

    printf("Duzenlenecek yazarin ID'sini girin: > ");
    if (scanf("%d", &duzenlenecek_id) != 1) {
        while (getchar() != '\n');
        printf("Gecersiz ID formati.\n");
        return;
    }
    while (getchar() != '\n');

    yazar_ptr = yazarBulNoIle(veri, duzenlenecek_id); // Yazarý bul.
    if (yazar_ptr == NULL) {
        printf("ID %d olan yazar bulunamadi.\n", duzenlenecek_id);
        return;
    }

    // Yeni adý al (boþ býrakýlýrsa deðiþmez).
    printf("Mevcut Ad: %s. Yeni adi girin (degistirmek istemiyorsaniz Enter'a basin): > ", yazar_ptr->adi);
    satirAl(stdin, yeni_ad_buffer, sizeof(yeni_ad_buffer));
    bosluklariSil(yeni_ad_buffer);
    if (strlen(yeni_ad_buffer) > 0) { // Eðer bir þey girildiyse.
        strncpy(yazar_ptr->adi, yeni_ad_buffer, MAX_AD_UZUNLUK -1);
        yazar_ptr->adi[MAX_AD_UZUNLUK-1] = '\0';
    }

    // Yeni soyadý al (boþ býrakýlýrsa deðiþmez).
    printf("Mevcut Soyad: %s. Yeni soyadi girin (degistirmek istemiyorsaniz Enter'a basin): > ", yazar_ptr->soyadi);
    satirAl(stdin, yeni_soyad_buffer, sizeof(yeni_soyad_buffer));
    bosluklariSil(yeni_soyad_buffer);
    if (strlen(yeni_soyad_buffer) > 0) {
        strncpy(yazar_ptr->soyadi, yeni_soyad_buffer, MAX_SOYAD_UZUNLUK-1);
        yazar_ptr->soyadi[MAX_SOYAD_UZUNLUK-1] = '\0';
    }

    yazarlariDosyayaYaz(veri); // Deðiþiklikleri kaydet.
    printf("Yazar (ID: %d) bilgileri guncellendi.\n", duzenlenecek_id);
}

// Verilen ID'deki yazarýn kitaplarýný listeliyor.
void yazarinKitaplariniListele(KutuphaneVerisi* veri, int yazarID_param){
    int i;
    int kitap_bulundu = 0;
    KitapBilgisi* kitap_ptr;
    YazarBilgisi* yazar_kontrol = yazarBulNoIle(veri, yazarID_param); // Yazar var mý diye kontrol et.

    if (!yazar_kontrol) {
        printf("ID %d olan yazar sistemde bulunmuyor.\n", yazarID_param);
        return;
    }

    printf("\n--- Yazar: %s %s (ID: %d) icin Kitaplar ---\n", yazar_kontrol->adi, yazar_kontrol->soyadi, yazarID_param);
    // Kitap-yazar iliþkilerini dolaþ.
    for(i=0; i < veri->kitapYazarAdedi; i++){
        if(veri->kitapYazarIliskileri[i].yazarID == yazarID_param){ // Yazar ID'si eþleþirse.
            kitap_ptr = kitapBulIsbnIle(veri, veri->kitapYazarIliskileri[i].kitabinIsbn); // Kitabý bul.
            if(kitap_ptr){ // Kitap bulunduysa.
                if(!kitap_bulundu) { // Ýlk kitapsa baþlýklarý yazdýr.
                    printf("%-15s %-30s\n", "ISBN", "Kitap Adi");
                    printf("-----------------------------------------------\n");
                }
                printf("%-15s %-30.30s\n", kitap_ptr->isbnNo, kitap_ptr->kitapAdi);
                kitap_bulundu = 1;
            } else {
                 fprintf(stderr, "Uyari: Yazar ID %d icin KitapYazar.csv'de bulunan %s ISBN'li kitap, kitap listesinde bulunamadi.\n", yazarID_param, veri->kitapYazarIliskileri[i].kitabinIsbn);
            }
        }
    }
    if(!kitap_bulundu){ // Hiç kitap bulunamadýysa.
        printf("Bu yazara ait kayitli kitap bulunmuyor.\n");
    }
    printf("-----------------------------------------------\n");
}

// Kullanýcýdan yazar ID'si alýp o yazarýn kitaplarýný listeliyor.
void yazarinKitaplariniGosterArayuz(KutuphaneVerisi* veri){
    int yazar_id_input;
    YazarBilgisi* yazar_ptr;
    printf("Kitaplarini goruntulemek istediginiz yazarin ID'sini girin: > ");
    if(scanf("%d", &yazar_id_input) != 1){
        while(getchar()!='\n');
        printf("Gecersiz yazar ID formati.\n");
        return;
    }
    while(getchar()!='\n');

    yazar_ptr = yazarBulNoIle(veri, yazar_id_input);
    if(yazar_ptr == NULL){
        printf("ID %d olan yazar bulunamadi (arayuz kontrolu).\n", yazar_id_input);
        return;
    }
    yazarinKitaplariniListele(veri, yazar_id_input);
}

// Yeni bir öðrenci düðümü oluþturup bilgilerini dolduruyor.
OgrenciBilgisi* ogrenciOlustur(const char* ogrenciNumarasi_param, const char* ad_param, const char* soyad_param, int puan_param) {
    OgrenciBilgisi* yeniOgrenci = (OgrenciBilgisi*)malloc(sizeof(OgrenciBilgisi));
    if (!yeniOgrenci) {
        perror("Yeni ogrenci icin bellek ayrilamadi");
        return NULL;
    }
    strncpy(yeniOgrenci->ogrNo, ogrenciNumarasi_param, OGRENCI_NO_UZUNLUK - 1);
    yeniOgrenci->ogrNo[OGRENCI_NO_UZUNLUK - 1] = '\0';
    strncpy(yeniOgrenci->adi, ad_param, MAX_AD_UZUNLUK -1);
    yeniOgrenci->adi[MAX_AD_UZUNLUK-1] = '\0';
    strncpy(yeniOgrenci->soyadi, soyad_param, MAX_SOYAD_UZUNLUK-1);
    yeniOgrenci->soyadi[MAX_SOYAD_UZUNLUK-1] = '\0';
    yeniOgrenci->puan = puan_param;
    yeniOgrenci->sonrakiOgrenci = NULL;
    yeniOgrenci->oncekiOgrenci = NULL;
    return yeniOgrenci;
}

// Yeni öðrenciyi listenin sonuna ekliyor. Ayný numaralý öðrenci varsa eklemiyor.
int ogrenciyiListeyeEkle(KutuphaneVerisi* veri, OgrenciBilgisi* yeniOgrenci_param) {
    OgrenciBilgisi* mevcut;
    if (yeniOgrenci_param == NULL) return 0;

    if (ogrenciBulNoIle(veri, yeniOgrenci_param->ogrNo) != NULL) {
        printf("Hata: %s numarali ogrenci zaten mevcut. Eklenemiyor.\n", yeniOgrenci_param->ogrNo);
        free(yeniOgrenci_param);
        return 0;
    }

    if (!veri->ilkOgrenci) {
        veri->ilkOgrenci = yeniOgrenci_param;
    } else {
        mevcut = veri->ilkOgrenci;
        while (mevcut->sonrakiOgrenci != NULL) {
            mevcut = mevcut->sonrakiOgrenci;
        }
        mevcut->sonrakiOgrenci = yeniOgrenci_param;
        yeniOgrenci_param->oncekiOgrenci = mevcut;
    }
    return 1;
}

// Kullanýcýdan öðrenci bilgilerini alýp ekleme iþlemini yapýyor.
void ogrenciEkleArayuz(KutuphaneVerisi* veri) {
    char ogrenciNo_buffer[OGRENCI_NO_UZUNLUK];
    char ad_buffer[MAX_AD_UZUNLUK];
    char soyad_buffer[MAX_SOYAD_UZUNLUK];
    OgrenciBilgisi* ogr_ptr;

    printf("8 haneli ogrenci numarasini girin: > ");
    satirAl(stdin, ogrenciNo_buffer, sizeof(ogrenciNo_buffer));
    bosluklariSil(ogrenciNo_buffer);

    if (strlen(ogrenciNo_buffer) != OGRENCI_NO_UZUNLUK -1) {
        printf("Ogrenci numarasi %d haneli olmalidir.\n", OGRENCI_NO_UZUNLUK -1);
        return;
    }

    printf("Ogrenci adini girin: > ");
    satirAl(stdin, ad_buffer, sizeof(ad_buffer));
    bosluklariSil(ad_buffer);

    printf("Ogrenci soyadini girin: > ");
    satirAl(stdin, soyad_buffer, sizeof(soyad_buffer));
    bosluklariSil(soyad_buffer);

    if (strlen(ad_buffer) == 0 || strlen(soyad_buffer) == 0) {
        printf("Ogrenci adi ve soyadi bos olamaz.\n");
        return;
    }

    ogr_ptr = ogrenciOlustur(ogrenciNo_buffer, ad_buffer, soyad_buffer, 100);
    if (!ogr_ptr) {
        return;
    }

    if (ogrenciyiListeyeEkle(veri, ogr_ptr)) {
        ogrencileriDosyayaYaz(veri);
        printf("Ogrenci %s %s (%s) basariyla eklendi ve kaydedildi.\n", ad_buffer, soyad_buffer, ogrenciNo_buffer);
    }
}

// Verilen numaraya sahip öðrenciyi bulup döndürüyor.
OgrenciBilgisi* ogrenciBulNoIle(KutuphaneVerisi* veri, const char* ogrenciNumarasi_param) {
    OgrenciBilgisi* mevcut = veri->ilkOgrenci;
    char temp_ogrenci_no[OGRENCI_NO_UZUNLUK];

    if (!ogrenciNumarasi_param || strlen(ogrenciNumarasi_param) == 0) return NULL;

    strncpy(temp_ogrenci_no, ogrenciNumarasi_param, OGRENCI_NO_UZUNLUK - 1);
    temp_ogrenci_no[OGRENCI_NO_UZUNLUK - 1] = '\0';
    bosluklariSil(temp_ogrenci_no);

    while(mevcut != NULL) {
        if(strcmp(mevcut->ogrNo, temp_ogrenci_no) == 0) {
            return mevcut;
        }
        mevcut = mevcut->sonrakiOgrenci;
    }
    return NULL;
}

// Kullanýcýdan numara alýp öðrenciyi siliyor. Öðrencinin üzerinde kitap varsa silmiyor.
void ogrenciSilArayuz(KutuphaneVerisi* veri) {
    char silinecek_id_input[OGRENCI_NO_UZUNLUK + 2];
    char silinecek_id_trimmed[OGRENCI_NO_UZUNLUK];
    OgrenciBilgisi* ogr_ptr;
    KitapBilgisi* k_iter;
    KitapKopyasi* kk_iter;
    int odunc_aktif = 0;
    char onay[4];

    printf("Silinecek ogrencinin numarasini girin: > ");
    satirAl(stdin, silinecek_id_input, sizeof(silinecek_id_input));

    strncpy(silinecek_id_trimmed, silinecek_id_input, OGRENCI_NO_UZUNLUK - 1);
    silinecek_id_trimmed[OGRENCI_NO_UZUNLUK - 1] = '\0';
    bosluklariSil(silinecek_id_trimmed);

    if (strlen(silinecek_id_trimmed) == 0) {
        printf("Ogrenci numarasi bos olamaz.\n");
        return;
    }
    if (strlen(silinecek_id_trimmed) != OGRENCI_NO_UZUNLUK -1) {
        printf("Ogrenci numarasi %d haneli olmalidir. Girilen: '%s'\n", OGRENCI_NO_UZUNLUK-1, silinecek_id_trimmed);
        return;
    }

    ogr_ptr = ogrenciBulNoIle(veri, silinecek_id_trimmed);
    if (ogr_ptr == NULL) {
        printf("Ogrenci %s bulunamadi.\n", silinecek_id_trimmed);
        return;
    }

    // Öðrencinin üzerinde kitap olup olmadýðýný kontrol et.
    k_iter = veri->ilkKitap;
    while(k_iter != NULL && !odunc_aktif){
        kk_iter = k_iter->kitapKopyalari;
        while(kk_iter != NULL && !odunc_aktif){
            if(strcmp(kk_iter->durumu, ogr_ptr->ogrNo) == 0){
                odunc_aktif = 1;
                printf("Hata: Ogrenci %s (%s %s) uzerinde '%s' etiketli kitap (%s) odunc alinmis durumda. Silinemez.\n",
                       ogr_ptr->ogrNo, ogr_ptr->adi, ogr_ptr->soyadi, kk_iter->kopyaEtiketi, k_iter->kitapAdi);
            }
            if (!odunc_aktif) {
                 kk_iter = kk_iter->sonrakiKopya;
            }
        }
        if(!odunc_aktif) {
            k_iter = k_iter->sonrakiKitap;
        }
    }

    if(odunc_aktif){
        return;
    }

    printf("Ogrenci %s %s (%s) silinecek. Emin misiniz? (E/H): > ", ogr_ptr->adi, ogr_ptr->soyadi, ogr_ptr->ogrNo);
    satirAl(stdin, onay, sizeof(onay));
    bosluklariSil(onay);

    if (strcmp(onay, "E") != 0 && strcmp(onay, "e") != 0) {
        printf("Ogrenci silme islemi iptal edildi.\n");
        return;
    }

    // Öðrenciyi çift yönlü listeden sil.
    if (ogr_ptr->oncekiOgrenci == NULL) {
        veri->ilkOgrenci = ogr_ptr->sonrakiOgrenci;
        if (veri->ilkOgrenci != NULL) {
            veri->ilkOgrenci->oncekiOgrenci = NULL;
        }
    } else {
        ogr_ptr->oncekiOgrenci->sonrakiOgrenci = ogr_ptr->sonrakiOgrenci;
        if (ogr_ptr->sonrakiOgrenci != NULL) {
            ogr_ptr->sonrakiOgrenci->oncekiOgrenci = ogr_ptr->oncekiOgrenci;
        }
    }
    free(ogr_ptr);
    ogrencileriDosyayaYaz(veri);
    printf("Ogrenci (%s) basariyla silindi.\n", silinecek_id_trimmed);
}

// Kullanýcýdan numara alýp öðrenci bilgilerini düzenliyor.
void ogrenciDuzenleArayuz(KutuphaneVerisi* veri) {
    char duzenlenecek_id_input[OGRENCI_NO_UZUNLUK + 2];
    char duzenlenecek_id_trimmed[OGRENCI_NO_UZUNLUK];
    OgrenciBilgisi* ogr_ptr;
    char yeni_ad_buffer[MAX_AD_UZUNLUK];
    char yeni_soyad_buffer[MAX_SOYAD_UZUNLUK];

    printf("Duzenlenecek ogrencinin numarasini girin: > ");
    satirAl(stdin, duzenlenecek_id_input, sizeof(duzenlenecek_id_input));
    strncpy(duzenlenecek_id_trimmed, duzenlenecek_id_input, OGRENCI_NO_UZUNLUK -1);
    duzenlenecek_id_trimmed[OGRENCI_NO_UZUNLUK-1] = '\0';
    bosluklariSil(duzenlenecek_id_trimmed);

    if (strlen(duzenlenecek_id_trimmed) != OGRENCI_NO_UZUNLUK -1) {
        printf("Ogrenci numarasi %d haneli olmalidir. Girilen: '%s'\n", OGRENCI_NO_UZUNLUK-1, duzenlenecek_id_trimmed);
        return;
    }

    ogr_ptr = ogrenciBulNoIle(veri, duzenlenecek_id_trimmed);
    if (ogr_ptr == NULL) {
        printf("Ogrenci %s bulunamadi.\n", duzenlenecek_id_trimmed);
        return;
    }

    // Yeni adý al.
    printf("Mevcut Ad: %s. Yeni adi girin (degistirmek istemiyorsaniz Enter'a basin): > ", ogr_ptr->adi);
    satirAl(stdin, yeni_ad_buffer, sizeof(yeni_ad_buffer));
    bosluklariSil(yeni_ad_buffer);
    if (strlen(yeni_ad_buffer) > 0) {
        strncpy(ogr_ptr->adi, yeni_ad_buffer, MAX_AD_UZUNLUK - 1);
        ogr_ptr->adi[MAX_AD_UZUNLUK -1] = '\0';
    }

    // Yeni soyadý al.
    printf("Mevcut Soyad: %s. Yeni soyadi girin (degistirmek istemiyorsaniz Enter'a basin): > ", ogr_ptr->soyadi);
    satirAl(stdin, yeni_soyad_buffer, sizeof(yeni_soyad_buffer));
    bosluklariSil(yeni_soyad_buffer);
    if (strlen(yeni_soyad_buffer) > 0) {
        strncpy(ogr_ptr->soyadi, yeni_soyad_buffer, MAX_SOYAD_UZUNLUK-1);
        ogr_ptr->soyadi[MAX_SOYAD_UZUNLUK-1] = '\0';
    }

    ogrencileriDosyayaYaz(veri);
    printf("Ogrenci (%s) bilgileri guncellendi.\n", duzenlenecek_id_trimmed);
}

// Verilen öðrenci ID'sine ait tüm ödünç alma ve iade geçmiþini listeliyor.
void ogrencininOduncGecmisiniGoster(KutuphaneVerisi* veri, const char* ogrenciID_str_param) {
    OduncIslemi* kayit_ptr = veri->ilkOduncKaydi;
    int gecmis_bulundu = 0;
    char temp_ogrenci_id[OGRENCI_NO_UZUNLUK];

    if (!ogrenciID_str_param) return;
    strncpy(temp_ogrenci_id, ogrenciID_str_param, OGRENCI_NO_UZUNLUK -1);
    temp_ogrenci_id[OGRENCI_NO_UZUNLUK-1] = '\0';
    bosluklariSil(temp_ogrenci_id);

    printf("\n--- Ogrenci %s Odunc Gecmisi ---\n", temp_ogrenci_id);
    printf("%-30s %-15s %-12s\n", "Kitap Etiket No", "Islem Turu", "Tarih");
    printf("------------------------------------------------------------\n");

    // Tüm ödünç kayýtlarýný dolaþ.
    while (kayit_ptr != NULL) {
        if (strcmp(kayit_ptr->oduncAlanOgrNo, temp_ogrenci_id) == 0) {
            printf("%-30s %-15s %-12s\n",
                   kayit_ptr->oduncKitapEtiketi,
                   (kayit_ptr->islemTipi == 0) ? "Odunc Alma" : "Iade Etme",
                   kayit_ptr->islemTarihi);
            gecmis_bulundu = 1;
        }
        kayit_ptr = kayit_ptr->sonrakiOduncIslemi;
    }

    if (!gecmis_bulundu) {
        printf("Bu ogrenciye ait odunc kaydi bulunmuyor.\n");
    }
    printf("------------------------------------------------------------\n");
}

// Kullanýcýdan öðrenci ID'si alýp o öðrencinin bilgilerini ve ödünç geçmiþini gösteriyor.
void ogrenciDetaylariniGosterArayuz(KutuphaneVerisi* veri) {
    char ogr_id_input[OGRENCI_NO_UZUNLUK + 2];
    char ogr_id_trimmed[OGRENCI_NO_UZUNLUK];
    OgrenciBilgisi* ogr_ptr;

    printf("Bilgilerini ve odunc gecmisini goruntulemek istediginiz ogrencinin numarasini girin: > ");
    satirAl(stdin, ogr_id_input, sizeof(ogr_id_input));
    strncpy(ogr_id_trimmed, ogr_id_input, OGRENCI_NO_UZUNLUK -1);
    ogr_id_trimmed[OGRENCI_NO_UZUNLUK-1] = '\0';
    bosluklariSil(ogr_id_trimmed);

     if (strlen(ogr_id_trimmed) != OGRENCI_NO_UZUNLUK -1) {
        printf("Ogrenci numarasi %d haneli olmalidir. Girilen: '%s'\n", OGRENCI_NO_UZUNLUK-1, ogr_id_trimmed);
        return;
    }

    ogr_ptr = ogrenciBulNoIle(veri, ogr_id_trimmed);
    if (ogr_ptr == NULL) {
        printf("Ogrenci %s bulunamadi.\n", ogr_id_trimmed);
        return;
    }

    // Öðrenci bilgilerini yazdýr.
    printf("\n--- Ogrenci Bilgileri ---\n");
    printf("Numara: %s\n", ogr_ptr->ogrNo);
    printf("Ad: %s\n", ogr_ptr->adi);
    printf("Soyad: %s\n", ogr_ptr->soyadi);
    printf("Puan: %d\n", ogr_ptr->puan);

    ogrencininOduncGecmisiniGoster(veri, ogr_ptr->ogrNo);
}

// Üzerinde kitap olan (yani iade etmemiþ) öðrencileri listeliyor.
void iadeEtmeyenOgrencileriGoster(KutuphaneVerisi* veri) {
    KitapBilgisi* k_iter = veri->ilkKitap;
    KitapKopyasi* kk_iter;
    OgrenciBilgisi* ogr_ptr;
    int bulundu = 0;

    printf("\n--- Kitabi Iade Etmemis Ogrenciler ---\n");
    printf("%-10s %-20s %-20s %-30s %-15s\n", "Ogr. No", "Ogr. Adi", "Ogr. Soyadi", "Kitap Etiketi", "Kitap Adi");
    printf("---------------------------------------------------------------------------------------------------\n");

    k_iter = veri->ilkKitap;
    // Tüm kitaplarý dolaþ.
    while (k_iter != NULL) {
        kk_iter = k_iter->kitapKopyalari;
        while (kk_iter != NULL) {
            // Eðer kopyanýn durumu "Rafta" deðilse ve boþ deðilse
            if (strcmp(kk_iter->durumu, "Rafta") != 0 && strlen(kk_iter->durumu) > 0) {
                ogr_ptr = ogrenciBulNoIle(veri, kk_iter->durumu);
                if (ogr_ptr) { // Öðrenci bulunduysa.
                    char k_ad_kisaltilmis[16];
                    if (strlen(k_iter->kitapAdi) > 15) {
                        strncpy(k_ad_kisaltilmis, k_iter->kitapAdi, 12);
                        k_ad_kisaltilmis[12] = '.';
                        k_ad_kisaltilmis[13] = '.';
                        k_ad_kisaltilmis[14] = '.';
                        k_ad_kisaltilmis[15] = '\0';
                    } else {
                        strncpy(k_ad_kisaltilmis, k_iter->kitapAdi, 15);
                        k_ad_kisaltilmis[15] = '\0';
                    }

                    printf("%-10s %-20.20s %-20.20s %-30s %-15.15s\n",
                           ogr_ptr->ogrNo, ogr_ptr->adi, ogr_ptr->soyadi,
                           kk_iter->kopyaEtiketi, k_ad_kisaltilmis);
                    bulundu = 1;
                } else {
                    fprintf(stderr, "Uyari: '%s' etiketli kitap, sistemde olmayan '%s' numarali ogrenciye odunc verilmis gorunuyor.\n", kk_iter->kopyaEtiketi, kk_iter->durumu);
                }
            }
            kk_iter = kk_iter->sonrakiKopya;
        }
        k_iter = k_iter->sonrakiKitap;
    }

    if (!bulundu) {
        printf("Su anda oduncte kitabi olan ogrenci bulunmuyor.\n");
    }
    printf("---------------------------------------------------------------------------------------------------\n");
}

// Puaný 0'dan küçük olan (yani cezalý) öðrencileri listeliyor.
void cezaliOgrencileriGoster(KutuphaneVerisi* veri) {
    OgrenciBilgisi* mevcut = veri->ilkOgrenci;
    int bulundu = 0;
    printf("\n--- Cezali Ogrenciler (Puan < 0) ---\n");
    printf("%-10s %-20s %-20s %-6s\n", "Ogr. No", "Ad", "Soyad", "Puan");
    printf("---------------------------------------------------------\n");

    // Tüm öðrencileri dolaþ.
    while (mevcut != NULL) {
        if (mevcut->puan < 0) {
            printf("%-10s %-20.20s %-20.20s %-6d\n", mevcut->ogrNo, mevcut->adi, mevcut->soyadi, mevcut->puan);
            bulundu = 1;
        }
        mevcut = mevcut->sonrakiOgrenci;
    }
    if(!bulundu){
        printf("Cezali ogrenci bulunmuyor.\n");
    }
    printf("---------------------------------------------------------\n");
}

// Yeni bir kitap düðümü oluþturup bilgilerini dolduruyor. Kopya sayýsý baþlangýçta 0.
KitapBilgisi* kitapOlustur(const char* isbn_param, const char* ad_param, int adet_param_kullanilmiyor) {
    KitapBilgisi* yeniKitap = (KitapBilgisi*)malloc(sizeof(KitapBilgisi));
    if (!yeniKitap) {
        perror("Kitap icin bellek ayrilamadi");
        return NULL;
    }
    strncpy(yeniKitap->isbnNo, isbn_param, ISBN_UZUNLUK -1);
    yeniKitap->isbnNo[ISBN_UZUNLUK-1] = '\0';
    strncpy(yeniKitap->kitapAdi, ad_param, 99);
    yeniKitap->kitapAdi[99] = '\0';
    yeniKitap->kopyaSayisi = 0;
    yeniKitap->kitapKopyalari = NULL;
    yeniKitap->sonrakiKitap = NULL;
    return yeniKitap;
}

// Yeni bir kitap kopyasý düðümü oluþturuyor. Etiketini ISBN_KopyaNo formatýnda ayarlýyor.
KitapKopyasi* yeniKitapKopyasiOlustur(const char* kitap_isbn_param, int kopya_numarasi, const char* durum_str_param) {
    KitapKopyasi* yeniKopya = (KitapKopyasi*)malloc(sizeof(KitapKopyasi));
    char temp_durum[OGRENCI_NO_UZUNLUK];

    if(!yeniKopya) {
        perror("KitapKopyasi icin bellek ayrilamadi");
        return NULL;
    }
    // Etiketi "ISBN_KopyaNo" þeklinde oluþtur.
    snprintf(yeniKopya->kopyaEtiketi, KITAP_ETIKET_UZUNLUK, "%s_%d", kitap_isbn_param, kopya_numarasi);

    strncpy(temp_durum, durum_str_param, OGRENCI_NO_UZUNLUK -1);
    temp_durum[OGRENCI_NO_UZUNLUK-1] = '\0';
    bosluklariSil(temp_durum);
    strcpy(yeniKopya->durumu, temp_durum);

    yeniKopya->sonrakiKopya = NULL;
    return yeniKopya;
}

// Yeni kitabý, kitap adýna göre (ve ayný adda olanlarý ISBN'e göre) sýralý bir þekilde listeye ekliyor.
// Ayný ISBN'li kitap varsa eklemiyor.
void kitabiListeyeEkle(KutuphaneVerisi* veri, KitapBilgisi* yeniKitap_param) {
    KitapBilgisi* mevcut;
    KitapKopyasi* gecici_kopya;
    KitapKopyasi* sonraki_k;

    if (!yeniKitap_param) {
        return;
    }

    // Ayný ISBN'li kitap var mý diye kontrol et.
    KitapBilgisi* bulunan_kitap_test = kitapBulIsbnIle(veri, yeniKitap_param->isbnNo);
    if (bulunan_kitap_test != NULL && bulunan_kitap_test != yeniKitap_param) {
        printf("Hata: ISBN %s olan kitap zaten mevcut. Eklenemiyor.\n", yeniKitap_param->isbnNo);
        // Eðer yeni kitapla kopyalar da oluþturulduysa onlarý serbest býrak.
        gecici_kopya = yeniKitap_param->kitapKopyalari;
        while(gecici_kopya){
            sonraki_k = gecici_kopya->sonrakiKopya;
            free(gecici_kopya);
            gecici_kopya = sonraki_k;
        }
        free(yeniKitap_param);
        return;
    }

    // Kitap adýna göre sýralý ekleme.
    if (veri->ilkKitap == NULL ||
        (strcmp(veri->ilkKitap->kitapAdi, yeniKitap_param->kitapAdi) > 0) ||
        (strcmp(veri->ilkKitap->kitapAdi, yeniKitap_param->kitapAdi) == 0 && strcmp(veri->ilkKitap->isbnNo, yeniKitap_param->isbnNo) > 0)) {
        yeniKitap_param->sonrakiKitap = veri->ilkKitap;
        veri->ilkKitap = yeniKitap_param;
    } else {
        mevcut = veri->ilkKitap;
        // Doðru yeri bulmak için listeyi dolaþ.
        while (mevcut->sonrakiKitap != NULL &&
               (strcmp(mevcut->sonrakiKitap->kitapAdi, yeniKitap_param->kitapAdi) < 0 ||
               (strcmp(mevcut->sonrakiKitap->kitapAdi, yeniKitap_param->kitapAdi) == 0 && strcmp(mevcut->sonrakiKitap->isbnNo, yeniKitap_param->isbnNo) < 0))) {
            mevcut = mevcut->sonrakiKitap;
        }
        // Araya eklerken ayný ISBN'li kitap var mý diye tekrar kontrol (normalde ilk kontrolde yakalanmalý).
        if (mevcut->sonrakiKitap && strcmp(mevcut->sonrakiKitap->isbnNo, yeniKitap_param->isbnNo) == 0) {
             printf("Hata: ISBN %s olan kitap zaten mevcut (araya ekleme sirasinda farkedildi). Eklenemiyor.\n", yeniKitap_param->isbnNo);
             gecici_kopya = yeniKitap_param->kitapKopyalari;
             while(gecici_kopya){
                 sonraki_k = gecici_kopya->sonrakiKopya;
                 free(gecici_kopya);
                 gecici_kopya = sonraki_k;
             }
             free(yeniKitap_param);
             return;
        }
        yeniKitap_param->sonrakiKitap = mevcut->sonrakiKitap;
        mevcut->sonrakiKitap = yeniKitap_param;
    }
}

// Kullanýcýdan kitap bilgilerini (ad, ISBN, kopya adedi) alýp ekleme iþlemini yapýyor.
// Kopyalarý da oluþturup kitaba baðlýyor.
void kitapEkleArayuz(KutuphaneVerisi* veri) {
    char ad_buffer[100];
    char isbn_giris[ISBN_UZUNLUK + 5];
    char isbn_islenmis[ISBN_UZUNLUK];
    int adet_kopya_input;
    KitapBilgisi* yeniKitap_ptr;
    KitapKopyasi* kopya_listesi_basi = NULL;
    KitapKopyasi* mevcut_kopya_kuyrugu = NULL;
    KitapKopyasi* kopya_ptr;
    int i;
    int kopya_olusturma_basarili = 1;

    printf("Kitap adini girin: > ");
    satirAl(stdin, ad_buffer, sizeof(ad_buffer));
    bosluklariSil(ad_buffer);

    printf("13 haneli ISBN girin (sadece rakamlar): > ");
    satirAl(stdin, isbn_giris, sizeof(isbn_giris));

    strncpy(isbn_islenmis, isbn_giris, ISBN_UZUNLUK - 1);
    isbn_islenmis[ISBN_UZUNLUK - 1] = '\0';
    bosluklariSil(isbn_islenmis);

    if (strlen(ad_buffer) == 0 || strlen(isbn_islenmis) == 0) {
        printf("Kitap adi ve ISBN bos olamaz.\n");
        return;
    }
    if (strlen(isbn_islenmis) != (ISBN_UZUNLUK - 1) ) {
        printf("ISBN tam olarak %d haneli rakam olmalidir. Girilen: '%s'\n", ISBN_UZUNLUK - 1, isbn_islenmis);
        return;
    }

    printf("Olusturulacak kopya adedini girin (pozitif bir sayi): > ");
    if (scanf("%d", &adet_kopya_input) != 1 || adet_kopya_input <= 0) {
        while(getchar()!='\n');
        printf("Gecersiz adet. Pozitif bir sayi girilmelidir.\n");
        return;
    }
    while(getchar()!='\n');

    yeniKitap_ptr = kitapOlustur(isbn_islenmis, ad_buffer, 0);
    if (!yeniKitap_ptr) return;

    // Ýstenen sayýda kopya oluþtur ve geçici listeye baðla.
    for (i = 1; i <= adet_kopya_input && kopya_olusturma_basarili; i++) {
        kopya_ptr = yeniKitapKopyasiOlustur(isbn_islenmis, i, "Rafta");
        if (kopya_ptr) {
            if (!kopya_listesi_basi) {
                kopya_listesi_basi = kopya_ptr;
                mevcut_kopya_kuyrugu = kopya_ptr;
            } else {
                mevcut_kopya_kuyrugu->sonrakiKopya = kopya_ptr;
                mevcut_kopya_kuyrugu = kopya_ptr;
            }
        } else {
            printf("HATA: Kitap kopyasi %d olusturulamadi. Kitap ekleme iptal ediliyor.\n", i);
            kopya_olusturma_basarili = 0;
        }
    }

    if (kopya_olusturma_basarili) {
        yeniKitap_ptr->kitapKopyalari = kopya_listesi_basi;
        yeniKitap_ptr->kopyaSayisi = adet_kopya_input;

        kitabiListeyeEkle(veri, yeniKitap_ptr);

        // Eklemenin baþarýlý olup olmadýðýný kontrol et (ISBN benzersizliði kitabiListeyeEkle'de kontrol ediliyor).
        if (kitapBulIsbnIle(veri, isbn_islenmis) == yeniKitap_ptr) {
            kitapAnaBilgileriniDosyayaYaz(veri);
            kitapKopyalariniDosyayaYaz(veri);
            printf("'%s' adli kitap (ISBN: %s) %d kopya ile basariyla eklendi.\n", ad_buffer, isbn_islenmis, adet_kopya_input);
        }
    } else {
        KitapKopyasi* temp_k = kopya_listesi_basi;
        while(temp_k){
            KitapKopyasi* sonraki_temp_k = temp_k->sonrakiKopya;
            free(temp_k);
            temp_k = sonraki_temp_k;
        }
        free(yeniKitap_ptr);
    }
}

KitapBilgisi* kitapBulIsbnIle(KutuphaneVerisi* veri, const char* isbn_param) {
    KitapBilgisi* mevcut = veri->ilkKitap;
    char temp_isbn[ISBN_UZUNLUK];

    if (!isbn_param || strlen(isbn_param) == 0) return NULL;

    strncpy(temp_isbn, isbn_param, ISBN_UZUNLUK - 1);
    temp_isbn[ISBN_UZUNLUK-1] = '\0';
    bosluklariSil(temp_isbn);

    if (strlen(temp_isbn) != (ISBN_UZUNLUK - 1)) {
        return NULL;
    }

    mevcut = veri->ilkKitap;
    while (mevcut != NULL) {
        if (strcmp(mevcut->isbnNo, temp_isbn) == 0) {
            return mevcut;
        }
        mevcut = mevcut->sonrakiKitap;
    }
    return NULL;
}

// Verilen kitabýn kopyalarý arasýnda durumu "Rafta" olan ilk kopyayý bulup döndürüyor.
KitapKopyasi* raftaUygunKopyaBul(KitapBilgisi* kitap_param) {
    KitapKopyasi* mevcut_kopya;
    KitapKopyasi* bulunan_kopya = NULL;
    if (!kitap_param) return NULL;

    mevcut_kopya = kitap_param->kitapKopyalari;
    while(mevcut_kopya != NULL && bulunan_kopya == NULL) {
        if (strcmp(mevcut_kopya->durumu, "Rafta") == 0) {
            bulunan_kopya = mevcut_kopya; // Kopyayý bulduk.
        }
        if (bulunan_kopya == NULL) {
            mevcut_kopya = mevcut_kopya->sonrakiKopya;
        }
    }
    return bulunan_kopya;
}

// Kullanýcýdan ISBN alýp kitabý ve tüm kopyalarýný siliyor.
// Kitabýn herhangi bir kopyasý ödünçteyse silmiyor.
// Kitap-Yazar iliþkilerinden de bu kitaba ait kayýtlarý siliyor.
void kitapSilArayuz(KutuphaneVerisi* veri) {
    char silinecek_isbn_input[ISBN_UZUNLUK + 5];
    char silinecek_isbn_trimmed[ISBN_UZUNLUK];
    KitapBilgisi* mevcut_kitap_ptr = NULL;
    KitapBilgisi* onceki_kitap_ptr = NULL;
    KitapKopyasi* kopya_iter;
    KitapKopyasi* sonraki_k;
    int i = 0;
    int j;
    int odunc_verilmis = 0;
    char onay[4];
    int kitap_bulundu_flag = 0;

    printf("Silinecek kitabin 13 haneli ISBN'sini girin (sadece rakamlar): > ");
    satirAl(stdin, silinecek_isbn_input, sizeof(silinecek_isbn_input));

    strncpy(silinecek_isbn_trimmed, silinecek_isbn_input, ISBN_UZUNLUK - 1);
    silinecek_isbn_trimmed[ISBN_UZUNLUK - 1] = '\0';
    bosluklariSil(silinecek_isbn_trimmed);

    if (strlen(silinecek_isbn_trimmed) == 0) {
        printf("ISBN bos olamaz.\n");
        return;
    }
    if (strlen(silinecek_isbn_trimmed) != (ISBN_UZUNLUK - 1)) {
        printf("ISBN tam olarak %d haneli rakam olmalidir. Girilen: '%s'\n", ISBN_UZUNLUK - 1, silinecek_isbn_trimmed);
        return;
    }

    // Silinecek kitabý bul.
    mevcut_kitap_ptr = veri->ilkKitap;
    while(mevcut_kitap_ptr != NULL && !kitap_bulundu_flag) {
        if (strcmp(mevcut_kitap_ptr->isbnNo, silinecek_isbn_trimmed) == 0) {
            kitap_bulundu_flag = 1;
        } else {
            onceki_kitap_ptr = mevcut_kitap_ptr;
            mevcut_kitap_ptr = mevcut_kitap_ptr->sonrakiKitap;
        }
    }

    if (!kitap_bulundu_flag) {
        printf("ISBN '%s' olan kitap bulunamadi.\n", silinecek_isbn_trimmed);
        return;
    }

    // Kopyalarýn ödünçte olup olmadýðýný kontrol et.
    kopya_iter = mevcut_kitap_ptr->kitapKopyalari;
    while (kopya_iter != NULL && !odunc_verilmis) {
        if (strcmp(kopya_iter->durumu, "Rafta") != 0) {
            odunc_verilmis = 1;
        }
        if (!odunc_verilmis) {
            kopya_iter = kopya_iter->sonrakiKopya;
        }
    }

    if (odunc_verilmis) {
        printf("Kitap '%s' (ISBN: %s) su anda bir veya daha fazla kopyasi oduncte oldugu icin silinemez.\n", mevcut_kitap_ptr->kitapAdi, mevcut_kitap_ptr->isbnNo);
        printf("Lutfen once tum kopyalarin iade edildiginden emin olun.\n");
        return;
    }

    printf("Kitap '%s' (ISBN: %s) ve tum kopyalari silinecek. Ayrica bu kitapla iliskili tum yazar atamalari da kaldirilacak. Emin misiniz? (E/H): > ", mevcut_kitap_ptr->kitapAdi, mevcut_kitap_ptr->isbnNo);
    satirAl(stdin, onay, sizeof(onay));
    bosluklariSil(onay);
    if (strcmp(onay, "E") != 0 && strcmp(onay, "e") != 0) {
        printf("Kitap silme islemi iptal edildi.\n");
        return;
    }

    // Kitabýn tüm kopyalarýný sil (bellekten).
    kopya_iter = mevcut_kitap_ptr->kitapKopyalari;
    while (kopya_iter != NULL) {
        sonraki_k = kopya_iter->sonrakiKopya;
        free(kopya_iter);
        kopya_iter = sonraki_k;
    }
    mevcut_kitap_ptr->kitapKopyalari = NULL;
    mevcut_kitap_ptr->kopyaSayisi = 0;

    // Kitap-Yazar iliþkilerinden bu kitaba ait olanlarý sil.
    // Bu biraz tricky, diziden eleman silmek için kaydýrma yapýlýyor.
    i = 0;
    while (i < veri->kitapYazarAdedi) {
        if (strcmp(veri->kitapYazarIliskileri[i].kitabinIsbn, silinecek_isbn_trimmed) == 0) {
            for (j = i; j < veri->kitapYazarAdedi - 1; j++) {
                veri->kitapYazarIliskileri[j] = veri->kitapYazarIliskileri[j + 1];
            }
            veri->kitapYazarAdedi--;
        } else {
            i++;
        }
    }
    // Eðer eleman silindiyse ve dizi boþ deðilse, realloc ile diziyi küçült.
    if (veri->kitapYazarAdedi > 0 && i > veri->kitapYazarAdedi) { // Bu koþul i > veri->kitapYazarAdedi hatalý olabilir, silme olduysa i'nin deðeri deðiþmeyecek, sadece kitapYazarAdedi azalacak.
                                                               // Doðrusu, silme iþlemi yapýldýysa ve kitapYazarAdedi > 0 ise realloc denenebilir.
                                                               // Veya en basiti, her silme sonrasý realloc denemek (performans azalsa da daha güvenli).
                                                               // Þimdilik orijinal mantýðý koruyorum, ancak bu kýsým gözden geçirilebilir.
        KitapYazarIliskisi* tempDizi = (KitapYazarIliskisi*)realloc(veri->kitapYazarIliskileri, veri->kitapYazarAdedi * sizeof(KitapYazarIliskisi));
        if (tempDizi == NULL && veri->kitapYazarAdedi > 0 ) {
            perror("Kitap-yazar iliskileri icin bellek yeniden tahsis edilemedi (kucultme), eski dizi kullanilmaya devam edilecek.");
        } else if (tempDizi != NULL || veri->kitapYazarAdedi == 0) {
           veri->kitapYazarIliskileri = tempDizi;
        }
    } else if (veri->kitapYazarAdedi == 0 && veri->kitapYazarIliskileri != NULL) {
        free(veri->kitapYazarIliskileri);
        veri->kitapYazarIliskileri = NULL;
    }


    // Kitabý ana listeden sil.
    if (onceki_kitap_ptr == NULL) {
        veri->ilkKitap = mevcut_kitap_ptr->sonrakiKitap;
    } else { // Ortada veya sondaysa.
        onceki_kitap_ptr->sonrakiKitap = mevcut_kitap_ptr->sonrakiKitap;
    }
    free(mevcut_kitap_ptr);

    // Dosyalarý güncelle.
    kitapAnaBilgileriniDosyayaYaz(veri);
    kitapKopyalariniDosyayaYaz(veri);
    kitapYazarIliskileriniDosyayaYaz(veri);

    printf("Kitap (ISBN: %s) ve iliskili tum kayitlari basariyla silindi.\n", silinecek_isbn_trimmed);
}

// Kullanýcýdan ISBN alýp kitabýn adýný düzenliyor.
// DÝKKAT: Kitap adý deðiþirse sýralý liste bozulabilir! Bu fonksiyon geliþtirilmeli.
void kitapDuzenleArayuz(KutuphaneVerisi* veri) {
    char isbn_duzenle_input[ISBN_UZUNLUK + 5];
    char isbn_duzenle_trimmed[ISBN_UZUNLUK];
    KitapBilgisi* kitap_ptr;
    char yeni_ad_buffer[100];

    printf("Duzenlenecek kitabin ISBN'sini girin (sadece rakamlar): > ");
    satirAl(stdin, isbn_duzenle_input, sizeof(isbn_duzenle_input));
    strncpy(isbn_duzenle_trimmed, isbn_duzenle_input, ISBN_UZUNLUK -1);
    isbn_duzenle_trimmed[ISBN_UZUNLUK-1] = '\0';
    bosluklariSil(isbn_duzenle_trimmed);

    if (strlen(isbn_duzenle_trimmed) != (ISBN_UZUNLUK - 1)) {
        printf("ISBN tam olarak %d haneli rakam olmalidir. Girilen: '%s'\n", ISBN_UZUNLUK - 1, isbn_duzenle_trimmed);
        return;
    }

    kitap_ptr = kitapBulIsbnIle(veri, isbn_duzenle_trimmed);
    if (kitap_ptr == NULL) {
        printf("ISBN %s olan kitap bulunamadi.\n", isbn_duzenle_trimmed);
        return;
    }

    printf("Mevcut Kitap Adi: %s\n", kitap_ptr->kitapAdi);
    printf("Yeni kitap adini girin (degistirmek istemiyorsaniz Enter'a basin): > ");
    satirAl(stdin, yeni_ad_buffer, sizeof(yeni_ad_buffer));
    bosluklariSil(yeni_ad_buffer);

    if (strlen(yeni_ad_buffer) > 0) {
        printf("UYARI: Kitap adi degistirilirse liste siralamasi bozulabilir. Bu islev gelistirilmelidir.\n");
        strncpy(kitap_ptr->kitapAdi, yeni_ad_buffer, 99);
        kitap_ptr->kitapAdi[99] = '\0';
        kitapAnaBilgileriniDosyayaYaz(veri);
        printf("Kitap (ISBN: %s) adi guncellendi. Liste siralamasi kontrol edilmeli.\n", isbn_duzenle_trimmed);
    } else {
        printf("Kitap adi degistirilmedi.\n");
    }
}

// Tüm kitaplarý ve her kitabýn kopyalarýný (etiket ve durumlarýyla) listeliyor.
void butunKitapVeKopyalariGoster(KutuphaneVerisi* veri) {
    KitapBilgisi* mevcut_kitap_ptr = veri->ilkKitap;
    KitapKopyasi* mevcut_kopya_ptr;
    int kitap_sayisi_local = 0;

    printf("\n--- Tum Kitaplarin ve Kopyalarinin Listesi ---\n");
    if (!mevcut_kitap_ptr) {
        printf("Sistemde kitap bulunmuyor.\n");
        printf("==========================================================================================\n");
        return;
    }
    printf("%-30s %-15s %-5s %s\n", "Kitap Adi", "ISBN", "Adet", "Kopyalar (Etiket - Durum)");
    printf("==========================================================================================\n");
    // Tüm kitaplarý dolaþ.
    while (mevcut_kitap_ptr != NULL) {
        kitap_sayisi_local++;
        char k_ad_kisaltilmis[31];
        strncpy(k_ad_kisaltilmis, mevcut_kitap_ptr->kitapAdi, 30);
        k_ad_kisaltilmis[30] = '\0';

        printf("%-30.30s %-15s %-5d ", k_ad_kisaltilmis, mevcut_kitap_ptr->isbnNo, mevcut_kitap_ptr->kopyaSayisi);
        mevcut_kopya_ptr = mevcut_kitap_ptr->kitapKopyalari;
        if (!mevcut_kopya_ptr) {
            printf("Listelenecek kopya yok.\n");
        } else {
            printf("\n");
            while (mevcut_kopya_ptr != NULL) {
                printf("                                     %-20s - %s\n", mevcut_kopya_ptr->kopyaEtiketi, mevcut_kopya_ptr->durumu);
                mevcut_kopya_ptr = mevcut_kopya_ptr->sonrakiKopya;
            }
        }
        printf("------------------------------------------------------------------------------------------\n");
        mevcut_kitap_ptr = mevcut_kitap_ptr->sonrakiKitap;
    }
    if (kitap_sayisi_local == 0 && veri->ilkKitap != NULL) {
         printf("UYARI: Kitap listesi (ilkKitap) dolu gorunuyor ancak hic kitap listelenemedi.\n");
         printf("------------------------------------------------------------------------------------------\n");
    }
}

// Durumu "Rafta" olan tüm kitap kopyalarýný listeliyor.
void raftakiKitaplariGoster(KutuphaneVerisi* veri) {
    KitapBilgisi* k_iter = veri->ilkKitap;
    KitapKopyasi* kk_iter;
    int toplam_raftaki_kopya = 0;
    int kitap_basina_raftaki;

    printf("\n--- Raftaki Kitaplar Listesi ---\n");
    printf("%-30s %-15s %-20s\n", "Kitap Adi", "ISBN", "Raftaki Kopya Etiketi");
    printf("======================================================================\n");

    if (k_iter == NULL) {
        printf("Sistemde hic kitap bulunmuyor.\n");
        printf("----------------------------------------------------------------------\n");
        return;
    }

    k_iter = veri->ilkKitap;

    while (k_iter != NULL) {
        kk_iter = k_iter->kitapKopyalari;
        kitap_basina_raftaki = 0;
        // Tüm kopyalarý dolaþ.
        while (kk_iter != NULL) {
            if (strcmp(kk_iter->durumu, "Rafta") == 0) {
                if(kitap_basina_raftaki == 0){
                    char k_ad_kisaltilmis[31];
                    strncpy(k_ad_kisaltilmis, k_iter->kitapAdi, 30);
                    k_ad_kisaltilmis[30] = '\0';
                    printf("%-30.30s %-15s ", k_ad_kisaltilmis, k_iter->isbnNo);
                } else {
                    printf("%-30s %-15s ", "", "");
                }
                printf("%-20s\n", kk_iter->kopyaEtiketi);
                toplam_raftaki_kopya++;
                kitap_basina_raftaki++;
            }
            kk_iter = kk_iter->sonrakiKopya;
        }
        if(kitap_basina_raftaki > 0) {
             printf("----------------------------------------------------------------------\n");
        }
        k_iter = k_iter->sonrakiKitap;
    }

    if (toplam_raftaki_kopya == 0) {
        printf("Rafta hic kitap kopyasi bulunmuyor.\n");
        if (veri->ilkKitap != NULL) {
             printf("----------------------------------------------------------------------\n");
        }
    }
}

// Ödünç alýnmýþ ve iade süresi geçmiþ (MAX_ODUNC_SURESI'ni aþmýþ) kitaplarý listeliyor.
void gecikenKitaplariGoster(KutuphaneVerisi* veri) {
    KitapBilgisi* k_iter;
    KitapKopyasi* kk_iter;
    OduncIslemi* o_iter;
    char guncel_tarih_buffer[TARIH_UZUNLUK];
    int fark; // Gün farký.
    int gecikmis_bulundu = 0;
    OgrenciBilgisi* ogr_ptr;
    char ogr_ad_soyad_buffer[MAX_AD_UZUNLUK + MAX_SOYAD_UZUNLUK + 1];

    bugununTarihiniAl(guncel_tarih_buffer);
    printf("\n--- Zamaninda Teslim Edilmeyen (Gecikmis) Kitaplar (Sure > %d gun) ---\n", MAX_ODUNC_SURESI);
    printf("%-25s %-10s %-20s %-12s %-12s %-10s\n", "Kitap Etiketi", "Ogr. No", "Ogrenci Ad Soyad", "Alis Tarihi", "Bugun", "Gecikme(Gun)");
    printf("==============================================================================================================\n");

    k_iter = veri->ilkKitap;
    // Tüm kitaplarý dolaþ.
    while (k_iter != NULL) {
        kk_iter = k_iter->kitapKopyalari;
        // Tüm kopyalarý dolaþ.
        while (kk_iter != NULL) {
            if (strcmp(kk_iter->durumu, "Rafta") != 0 && strlen(kk_iter->durumu) == OGRENCI_NO_UZUNLUK -1) {
                OduncIslemi* son_odunc_bu_kopya_icin = NULL;
                o_iter = veri->ilkOduncKaydi;
                while(o_iter != NULL && son_odunc_bu_kopya_icin == NULL){
                    if(strcmp(o_iter->oduncKitapEtiketi, kk_iter->kopyaEtiketi) == 0 &&
                       strcmp(o_iter->oduncAlanOgrNo, kk_iter->durumu) == 0 &&
                       o_iter->islemTipi == 0) {
                        son_odunc_bu_kopya_icin = o_iter;
                    }
                    if (son_odunc_bu_kopya_icin == NULL) {
                        o_iter = o_iter->sonrakiOduncIslemi;
                    }
                }

                if (son_odunc_bu_kopya_icin) {
                    fark = gunFarkiniHesapla(son_odunc_bu_kopya_icin->islemTarihi, guncel_tarih_buffer);
                    if (fark > MAX_ODUNC_SURESI) {
                        ogr_ptr = ogrenciBulNoIle(veri, kk_iter->durumu);
                        strcpy(ogr_ad_soyad_buffer, "Bilinmiyor");
                        if(ogr_ptr) {
                            snprintf(ogr_ad_soyad_buffer, sizeof(ogr_ad_soyad_buffer), "%s %s", ogr_ptr->adi, ogr_ptr->soyadi);
                        }
                        char ogr_ad_soyad_kisaltilmis[21]; // Ad soyadý kýsalt.
                        strncpy(ogr_ad_soyad_kisaltilmis, ogr_ad_soyad_buffer, 20);
                        ogr_ad_soyad_kisaltilmis[20] = '\0';

                        printf("%-25s %-10s %-20.20s %-12s %-12s (%3d gun)\n",
                               kk_iter->kopyaEtiketi,
                               kk_iter->durumu,
                               ogr_ad_soyad_kisaltilmis,
                               son_odunc_bu_kopya_icin->islemTarihi,
                               guncel_tarih_buffer,
                               fark - MAX_ODUNC_SURESI); // Gecikme süresini yazdýr.
                        gecikmis_bulundu = 1;
                    }
                }
            }
            kk_iter = kk_iter->sonrakiKopya;
        }
        k_iter = k_iter->sonrakiKitap;
    }

    if (!gecikmis_bulundu) { // Hiç gecikmiþ kitap bulunamadýysa.
        printf("Gecikmis kitap bulunmuyor.\n");
    }
    printf("--------------------------------------------------------------------------------------------------------------\n");
}

// Verilen ISBN'deki kitaba, verilen ID'deki yazarý atýyor.
// Kitap-Yazar iliþkileri dizisine yeni bir eleman ekliyor.
// Yazar zaten o kitaba atanmýþsa bir þey yapmýyor.
void kitabaYazarAta(KutuphaneVerisi* veri, const char* kitapISBN_param, int eklenecekYazarID_param) {
    KitapYazarIliskisi* gecici_dizi; // realloc için.
    int i;
    YazarBilgisi* yzr_ptr; // Yazar kontrolü için.
    KitapBilgisi* ktp_ptr; // Kitap kontrolü için.
    char temp_isbn[ISBN_UZUNLUK];
    int zaten_atanmis = 0; // Yazar zaten atanmýþ mý bayraðý.

    if (!kitapISBN_param) return;
    strncpy(temp_isbn, kitapISBN_param, ISBN_UZUNLUK -1);
    temp_isbn[ISBN_UZUNLUK-1] = '\0';
    bosluklariSil(temp_isbn);

    ktp_ptr = kitapBulIsbnIle(veri, temp_isbn); // Kitap var mý?
    if (!ktp_ptr) {
        printf("Hata: ISBN %s olan kitap bulunamadi. Iliski olusturulamiyor.\n", temp_isbn);
        return;
    }
    yzr_ptr = yazarBulNoIle(veri, eklenecekYazarID_param); // Yazar var mý? (-1 silinmiþ yazar olabilir)
    if (!yzr_ptr && eklenecekYazarID_param != -1) {
        printf("Hata: ID %d olan yazar bulunamadi. Iliski olusturulamiyor.\n", eklenecekYazarID_param);
        return;
    }

    // Yazarýn zaten bu kitaba atanýp atanmadýðýný kontrol et.
    for(i=0; i < veri->kitapYazarAdedi && !zaten_atanmis; i++) {
        if(strcmp(veri->kitapYazarIliskileri[i].kitabinIsbn, temp_isbn) == 0 && veri->kitapYazarIliskileri[i].yazarID == eklenecekYazarID_param) {
            printf("Yazar %s %s (ID: %d) zaten %s ISBN'li kitaba ('%s') atanmis.\n", yzr_ptr ? yzr_ptr->adi : "Bilinmeyen", yzr_ptr ? yzr_ptr->soyadi : "Yazar", eklenecekYazarID_param, temp_isbn, ktp_ptr->kitapAdi);
            zaten_atanmis = 1;
        }
    }

    if (!zaten_atanmis) { // Eðer zaten atanmamýþsa.
        // Diziyi bir eleman büyüterek yeniden boyutlandýr.
        gecici_dizi = (KitapYazarIliskisi*)realloc(veri->kitapYazarIliskileri, (veri->kitapYazarAdedi + 1) * sizeof(KitapYazarIliskisi));
        if (!gecici_dizi) { // Bellek ayrýlamazsa.
            perror("Kitap-yazar iliskileri icin bellek yeniden ayrilamadi (buyutme)");
            return;
        }
        veri->kitapYazarIliskileri = gecici_dizi; // Yeni diziyi ata.

        // Yeni iliþkiyi dizinin sonuna ekle.
        strcpy(veri->kitapYazarIliskileri[veri->kitapYazarAdedi].kitabinIsbn, temp_isbn);
        veri->kitapYazarIliskileri[veri->kitapYazarAdedi].yazarID = eklenecekYazarID_param;
        veri->kitapYazarAdedi++; // Eleman sayýsýný artýr.
        printf("Kitap-Yazar iliskisi eklendi (Kitap: '%s' [ISBN: %s], Yazar: %s %s [ID: %d]).\n", ktp_ptr->kitapAdi, temp_isbn, yzr_ptr ? yzr_ptr->adi : "Bilinmeyen", yzr_ptr ? yzr_ptr->soyadi : "Yazar", eklenecekYazarID_param);
    }
}

// Kullanýcýdan kitap ISBN'i ve yazar ID'si alýp kitaba yazar atama iþlemini yapýyor.
void kitabaYazarAtaArayuz(KutuphaneVerisi* veri) {
    char isbn_input[ISBN_UZUNLUK + 5];
    char isbn_trimmed[ISBN_UZUNLUK];
    int yazar_id_input;
    KitapBilgisi* ktp_ptr;
    YazarBilgisi* yzr_ptr;

    printf("Yazar atanacak Kitap ISBN'sini girin (sadece rakamlar): > ");
    satirAl(stdin, isbn_input, sizeof(isbn_input));
    strncpy(isbn_trimmed, isbn_input, ISBN_UZUNLUK - 1);
    isbn_trimmed[ISBN_UZUNLUK - 1] = '\0';
    bosluklariSil(isbn_trimmed);

    if (strlen(isbn_trimmed) != (ISBN_UZUNLUK - 1)) {
        printf("ISBN tam olarak %d haneli rakam olmalidir. Girilen: '%s'\n", ISBN_UZUNLUK - 1, isbn_trimmed);
        return;
    }

    ktp_ptr = kitapBulIsbnIle(veri, isbn_trimmed); // Kitap var mý?
    if (!ktp_ptr) {
        printf("ISBN %s olan kitap bulunamadi.\n", isbn_trimmed);
        return;
    }

    printf("Kitaba ('%s') atanacak Yazar ID'sini girin: > ", ktp_ptr->kitapAdi);
    if (scanf("%d", &yazar_id_input) != 1) {
        while(getchar()!='\n');
        printf("Gecersiz Yazar ID.\n");
        return;
    }
    while(getchar()!='\n');

    yzr_ptr = yazarBulNoIle(veri, yazar_id_input); // Yazar var mý? (-1 olabilir)
    if (!yzr_ptr && yazar_id_input != -1) {
        printf("ID %d olan yazar bulunamadi.\n", yazar_id_input);
        return;
    }

    kitabaYazarAta(veri, isbn_trimmed, yazar_id_input); // Atama fonksiyonunu çaðýr.
    kitapYazarIliskileriniDosyayaYaz(veri); // Dosyayý güncelle.
}

// Verilen ISBN'deki kitabýn tüm yazarlarýný listeliyor.
void kitabinYazarlariniListele(KutuphaneVerisi* veri, const char* kitapISBN_param){
    int i;
    int yazar_bulundu = 0; // Bu kitaba atanmýþ yazar bulundu mu bayraðý.
    YazarBilgisi* yazar_ptr;
    KitapBilgisi* k_ptr; // Kitap kontrolü için.
    char temp_isbn[ISBN_UZUNLUK];

    if(!kitapISBN_param) return;
    strncpy(temp_isbn, kitapISBN_param, ISBN_UZUNLUK -1);
    temp_isbn[ISBN_UZUNLUK-1] = '\0';
    bosluklariSil(temp_isbn);

    k_ptr = kitapBulIsbnIle(veri, temp_isbn); // Kitap var mý?
    if(!k_ptr){
        printf("ISBN %s olan kitap bulunamadi (veya gecersiz formatta).\n", temp_isbn);
        return;
    }

    printf("\n--- Kitap: %s (ISBN: %s) Yazarlari ---\n", k_ptr->kitapAdi, k_ptr->isbnNo);
    printf("%-5s %-20s %-20s\n", "ID", "Ad", "Soyad");
    printf("----------------------------------------------\n");

    // Tüm kitap-yazar iliþkilerini dolaþ.
    for(i=0; i < veri->kitapYazarAdedi; i++){
        if(strcmp(veri->kitapYazarIliskileri[i].kitabinIsbn, k_ptr->isbnNo) == 0){ // ISBN eþleþirse.
            if(veri->kitapYazarIliskileri[i].yazarID == -1) { // Yazar ID'si -1 ise (silinmiþ yazar).
                 printf("%-5s %-20s %-20s\n", "(-1)", "(Silinmis Yazar)", "");
                 yazar_bulundu = 1;
            } else { // Yazar ID'si geçerliyse.
                yazar_ptr = yazarBulNoIle(veri, veri->kitapYazarIliskileri[i].yazarID); // Yazarý bul.
                if(yazar_ptr){ // Yazar bulunduysa.
                    printf("%-5d %-20s %-20s\n", yazar_ptr->yazarNo, yazar_ptr->adi, yazar_ptr->soyadi);
                    yazar_bulundu = 1;
                } else { // Yazar bulunamadýysa (KitapYazar.csv'de var ama Yazarlar.csv'de yok).
                    printf("(%d) (Bu ID'ye sahip yazar bulunamadi/silinmis)\n", veri->kitapYazarIliskileri[i].yazarID);
                    yazar_bulundu = 1;
                }
            }
        }
    }
    if(!yazar_bulundu){ // Hiç yazar bulunamadýysa.
        printf("Bu kitaba atanmis yazar bulunmuyor.\n");
    }
    printf("----------------------------------------------\n");
}

// Kullanýcýdan kitap ISBN'i alýp o kitaba yazar ekleme veya çýkarma iþlemi yapýyor.
void kitapYazarGuncelleArayuz(KutuphaneVerisi* veri) {
    char isbn_input[ISBN_UZUNLUK + 5];
    char isbn_trimmed[ISBN_UZUNLUK];
    int secim, yazar_id_eski, yazar_id_yeni;
    int i = 0;
    int j;
    int bulundu = 0; // Yazar bulundu mu (çýkarma iþlemi için).
    KitapYazarIliskisi* temp_dizi; // realloc için.
    KitapBilgisi* ktp_ptr; // Kitap kontrolü için.

    printf("Yazar(lar)ini guncellemek istediginiz kitabin ISBN'sini girin (sadece rakamlar): > ");
    satirAl(stdin, isbn_input, sizeof(isbn_input));
    strncpy(isbn_trimmed, isbn_input, ISBN_UZUNLUK - 1);
    isbn_trimmed[ISBN_UZUNLUK - 1] = '\0';
    bosluklariSil(isbn_trimmed);

    if (strlen(isbn_trimmed) != (ISBN_UZUNLUK - 1)) {
        printf("ISBN tam olarak %d haneli rakam olmalidir. Girilen: '%s'\n", ISBN_UZUNLUK - 1, isbn_trimmed);
        return;
    }

    ktp_ptr = kitapBulIsbnIle(veri, isbn_trimmed); // Kitap var mý?
    if (ktp_ptr == NULL) {
        printf("ISBN %s olan kitap bulunamadi.\n", isbn_trimmed);
        return;
    }

    kitabinYazarlariniListele(veri, isbn_trimmed); // Mevcut yazarlarý göster.

    printf("\nKitap-Yazar Guncelleme Secenekleri ('%s' icin):\n", ktp_ptr->kitapAdi);
    printf("1. Bu kitaba yeni yazar ekle\n");
    printf("2. Bu kitaptan yazar cikar\n");
    printf("0. Islemi iptal et\n");
    printf("Seciminiz: > ");
    if (scanf("%d", &secim) != 1) {
        secim = -1;
    }
    while (getchar() != '\n');

    switch (secim) {
        case 1: // Yeni yazar ekle.
            printf("Eklenecek Yazar ID'sini girin: > ");
            if (scanf("%d", &yazar_id_yeni) != 1) {
                while (getchar() != '\n'); printf("Gecersiz ID.\n"); return;
            }
            while (getchar() != '\n');
            if (!yazarBulNoIle(veri, yazar_id_yeni) && yazar_id_yeni != -1) { // Yazar var mý? (-1 olabilir)
                printf("ID %d olan yazar bulunamadi.\n", yazar_id_yeni); return;
            }
            kitabaYazarAta(veri, isbn_trimmed, yazar_id_yeni); // Atama fonksiyonunu çaðýr.
            kitapYazarIliskileriniDosyayaYaz(veri); // Dosyayý güncelle.
            break;
        case 2: // Yazar çýkar.
            printf("Cikarilacak Yazar ID'sini girin (Kitabin yazarlari listesinden bakiniz): > ");
             if (scanf("%d", &yazar_id_eski) != 1) {
                while (getchar() != '\n'); printf("Gecersiz ID.\n"); return;
            }
            while (getchar() != '\n');

            bulundu = 0;
            i = 0;
            // Kitap-Yazar iliþkileri dizisinden ilgili kaydý bul ve sil.
            while (i < veri->kitapYazarAdedi) {
                if (strcmp(veri->kitapYazarIliskileri[i].kitabinIsbn, isbn_trimmed) == 0 && veri->kitapYazarIliskileri[i].yazarID == yazar_id_eski) {
                    bulundu = 1;
                    for (j = i; j < veri->kitapYazarAdedi - 1; j++) { // Kalanlarý kaydýr.
                        veri->kitapYazarIliskileri[j] = veri->kitapYazarIliskileri[j + 1];
                    }
                    veri->kitapYazarAdedi--; // Eleman sayýsýný azalt.
                } else {
                    i++;
                }
            }
            if (bulundu) { // Eðer yazar çýkarýldýysa.
                if (veri->kitapYazarAdedi == 0 && veri->kitapYazarIliskileri != NULL) { // Dizi boþaldýysa.
                    free(veri->kitapYazarIliskileri); veri->kitapYazarIliskileri = NULL;
                } else if (veri->kitapYazarAdedi > 0) { // Dizi hala doluysa realloc yap.
                    temp_dizi = (KitapYazarIliskisi*)realloc(veri->kitapYazarIliskileri, veri->kitapYazarAdedi * sizeof(KitapYazarIliskisi));
                    if(temp_dizi == NULL && veri->kitapYazarAdedi > 0) {
                         perror("Realloc hatasi (kucultme) - KitapYazar. Eski dizi kullanilmaya devam edilecek.");
                    } else if (temp_dizi != NULL || veri->kitapYazarAdedi == 0) {
                        veri->kitapYazarIliskileri = temp_dizi;
                    }
                }
                kitapYazarIliskileriniDosyayaYaz(veri); // Dosyayý güncelle.
                printf("Yazar ID %d, %s ISBN'li kitaptan ('%s') cikarildi.\n", yazar_id_eski, isbn_trimmed, ktp_ptr->kitapAdi);
            } else { // Yazar bulunamadýysa.
                printf("Belirtilen yazar ID (%d) bu kitap (%s) icin bulunamadi veya zaten cikarilmis.\n", yazar_id_eski, isbn_trimmed);
            }
            break;
        case 0:
            printf("Islem iptal edildi.\n");
            break;
        default:
            printf("Gecersiz secim.\n");
    }
}

// Yeni bir ödünç iþlemi kaydý oluþturuyor.
OduncIslemi* yeniOduncKaydiOlustur(const char* kitapEtiketNo_param, const char* ogrenciID_str_param, int islemTuru_val, const char* tarih_str_param) {
    OduncIslemi* yeniKayit = (OduncIslemi*)malloc(sizeof(OduncIslemi));
    if (!yeniKayit) {
        perror("OduncIslemi icin bellek ayrilamadi");
        return NULL;
    }
    strncpy(yeniKayit->oduncKitapEtiketi, kitapEtiketNo_param, KITAP_ETIKET_UZUNLUK - 1);
    yeniKayit->oduncKitapEtiketi[KITAP_ETIKET_UZUNLUK-1] = '\0';
    strncpy(yeniKayit->oduncAlanOgrNo, ogrenciID_str_param, OGRENCI_NO_UZUNLUK - 1);
    yeniKayit->oduncAlanOgrNo[OGRENCI_NO_UZUNLUK -1] = '\0';
    yeniKayit->islemTipi = islemTuru_val; // 0: Ödünç Alma, 1: Ýade Etme
    strncpy(yeniKayit->islemTarihi, tarih_str_param, TARIH_UZUNLUK - 1);
    yeniKayit->islemTarihi[TARIH_UZUNLUK-1] = '\0';
    yeniKayit->sonrakiOduncIslemi = NULL;
    return yeniKayit;
}

// Yeni ödünç kaydýný listenin baþýna ekliyor. (En son iþlem en baþta olur)
void oduncKaydiniListeyeEkle(KutuphaneVerisi* veri, OduncIslemi* yeniKayit_param) {
    if (!yeniKayit_param) return;
    yeniKayit_param->sonrakiOduncIslemi = veri->ilkOduncKaydi;
    veri->ilkOduncKaydi = yeniKayit_param;
}

// Kullanýcýdan öðrenci ID'si ve kitap ISBN'i alýp kitap ödünç alma iþlemini yapýyor.
// Öðrencinin puaný negatifse veya kitabýn rafta uygun kopyasý yoksa iþlem yapmýyor.
void kitapOduncAlArayuz(KutuphaneVerisi* veri) {
    char ogrenci_id_input[OGRENCI_NO_UZUNLUK + 2];
    char ogrenci_id_trimmed[OGRENCI_NO_UZUNLUK];
    char kitap_isbn_input[ISBN_UZUNLUK + 5];
    char kitap_isbn_trimmed[ISBN_UZUNLUK];
    OgrenciBilgisi* ogrenci_ptr; // Ödünç alacak öðrenci.
    KitapBilgisi* odunc_alinacak_kitap_ptr; // Ödünç alýnacak kitap.
    KitapKopyasi* uygun_kopya_ptr; // Ödünç alýnacak uygun kopya.
    char guncel_tarih_str_buffer[TARIH_UZUNLUK]; // Ýþlem tarihi (bugün).
    OduncIslemi* odunc_islemi_ptr; // Oluþturulacak ödünç kaydý.

    printf("Odunc almak icin Ogrenci ID'sini girin: > ");
    satirAl(stdin, ogrenci_id_input, sizeof(ogrenci_id_input));
    strncpy(ogrenci_id_trimmed, ogrenci_id_input, OGRENCI_NO_UZUNLUK -1);
    ogrenci_id_trimmed[OGRENCI_NO_UZUNLUK-1] = '\0';
    bosluklariSil(ogrenci_id_trimmed);

     if (strlen(ogrenci_id_trimmed) != OGRENCI_NO_UZUNLUK -1) {
        printf("Ogrenci numarasi %d haneli olmalidir. Girilen: '%s'\n", OGRENCI_NO_UZUNLUK-1, ogrenci_id_trimmed);
        return;
    }

    ogrenci_ptr = ogrenciBulNoIle(veri, ogrenci_id_trimmed); // Öðrenciyi bul.
    if (!ogrenci_ptr) {
        printf("ID %s olan ogrenci bulunamadi.\n", ogrenci_id_trimmed);
        return;
    }
    if (ogrenci_ptr->puan < 0) { // Puaný negatifse ödünç alamaz.
        printf("Ogrenci %s (%s %s) negatif puana (%d) sahip ve kitap odunc alamaz.\n", ogrenci_id_trimmed, ogrenci_ptr->adi, ogrenci_ptr->soyadi, ogrenci_ptr->puan);
        return;
    }

    printf("Odunc alinacak kitabin ISBN'sini girin (sadece rakamlar): > ");
    satirAl(stdin, kitap_isbn_input, sizeof(kitap_isbn_input));
    strncpy(kitap_isbn_trimmed, kitap_isbn_input, ISBN_UZUNLUK -1);
    kitap_isbn_trimmed[ISBN_UZUNLUK-1] = '\0';
    bosluklariSil(kitap_isbn_trimmed);

    if (strlen(kitap_isbn_trimmed) != (ISBN_UZUNLUK - 1)) {
        printf("ISBN tam olarak %d haneli rakam olmalidir. Girilen: '%s'\n", ISBN_UZUNLUK - 1, kitap_isbn_trimmed);
        return;
    }

    odunc_alinacak_kitap_ptr = kitapBulIsbnIle(veri, kitap_isbn_trimmed); // Kitabý bul.
    if (!odunc_alinacak_kitap_ptr) {
        printf("ISBN %s olan kitap bulunamadi.\n", kitap_isbn_trimmed);
        return;
    }

    uygun_kopya_ptr = raftaUygunKopyaBul(odunc_alinacak_kitap_ptr); // Rafta uygun kopya var mý?
    if (!uygun_kopya_ptr) {
        printf("'%s' (ISBN: %s) kitabinin tum kopyalari su anda odunc verilmis veya rafta degil.\n", odunc_alinacak_kitap_ptr->kitapAdi, kitap_isbn_trimmed);
        return;
    }

    // Kopyanýn durumunu öðrenci numarasý ile güncelle.
    strncpy(uygun_kopya_ptr->durumu, ogrenci_ptr->ogrNo, OGRENCI_NO_UZUNLUK -1);
    uygun_kopya_ptr->durumu[OGRENCI_NO_UZUNLUK-1] = '\0';

    bugununTarihiniAl(guncel_tarih_str_buffer); // Bugünün tarihini al.
    // Yeni ödünç kaydý oluþtur (iþlem tipi 0: Ödünç Alma).
    odunc_islemi_ptr = yeniOduncKaydiOlustur(uygun_kopya_ptr->kopyaEtiketi, ogrenci_ptr->ogrNo, 0, guncel_tarih_str_buffer);
    if (odunc_islemi_ptr) { // Kayýt oluþturma baþarýlýysa.
        oduncKaydiniListeyeEkle(veri, odunc_islemi_ptr); // Kaydý listeye ekle.
        kitapKopyalariniDosyayaYaz(veri); // Kopyalarýn durumunu dosyaya yaz.
        oduncKayitlariniDosyayaYaz(veri); // Ödünç kayýtlarýný dosyaya yaz.
        printf("Kitap '%s' (Kopya: %s) ogrenci %s (%s %s) tarafindan %s tarihinde basariyla odunc alindi.\n",
               odunc_alinacak_kitap_ptr->kitapAdi, uygun_kopya_ptr->kopyaEtiketi,
               ogrenci_ptr->ogrNo, ogrenci_ptr->adi, ogrenci_ptr->soyadi, guncel_tarih_str_buffer);
    } else { // Kayýt oluþturma baþarýsýzsa.
        strcpy(uygun_kopya_ptr->durumu, "Rafta"); // Kopyanýn durumunu geri "Rafta" yap.
        printf("HATA: Odunc kaydi olusturulamadi. Islem iptal edildi.\n");
    }
}

// Kullanýcýdan öðrenci ID'si ve kitap etiket numarasý alýp kitap iade iþlemini yapýyor.
// Kitabýn ödünç alýnma tarihini bulup gecikme varsa ceza puaný uyguluyor.
void kitapIadeEtArayuz(KutuphaneVerisi* veri) {
    char ogrenci_id_input[OGRENCI_NO_UZUNLUK + 2];
    char ogrenci_id_trimmed[OGRENCI_NO_UZUNLUK];
    char iade_edilecek_kitap_etiketi_input[KITAP_ETIKET_UZUNLUK + 2];
    char iade_edilecek_kitap_etiketi_trimmed[KITAP_ETIKET_UZUNLUK];
    OgrenciBilgisi* ogrenci_ptr; // Ýade eden öðrenci.
    KitapBilgisi* ana_kitap_ptr = NULL; // Ýade edilen kitabýn ana bilgisi.
    KitapKopyasi* iade_edilecek_kopya_ptr = NULL; // Ýade edilecek kopya.
    KitapBilgisi* mevcut_b_iter; // Kitap listesi iteratörü.
    KitapKopyasi* mevcut_c_iter; // Kopya listesi iteratörü.
    char odunc_tarihi_str_buffer[TARIH_UZUNLUK] = ""; // Kitabýn ödünç alýndýðý tarih.
    OduncIslemi* lr_mevcut_iter; // Ödünç kayýtlarý iteratörü.
    OduncIslemi* son_odunc_kaydi_bu_kopya_icin = NULL; // Bu kopya için son ödünç alma kaydý.
    char guncel_tarih_str_buffer[TARIH_UZUNLUK]; // Ýþlem tarihi (bugün).
    int gun_farki_val; // Gün farký.
    OduncIslemi* iade_kaydi_ptr; // Oluþturulacak iade kaydý.
    int kopya_bulundu_ve_uygun = 0; // Ýade edilecek kopya bulundu ve öðrenciyle eþleþiyor mu bayraðý.

    printf("Iade icin Ogrenci ID'sini girin: > ");
    satirAl(stdin, ogrenci_id_input, sizeof(ogrenci_id_input));
    strncpy(ogrenci_id_trimmed, ogrenci_id_input, OGRENCI_NO_UZUNLUK -1);
    ogrenci_id_trimmed[OGRENCI_NO_UZUNLUK-1] = '\0';
    bosluklariSil(ogrenci_id_trimmed);

     if (strlen(ogrenci_id_trimmed) != OGRENCI_NO_UZUNLUK -1) {
        printf("Ogrenci numarasi %d haneli olmalidir. Girilen: '%s'\n", OGRENCI_NO_UZUNLUK-1, ogrenci_id_trimmed);
        return;
    }

    ogrenci_ptr = ogrenciBulNoIle(veri, ogrenci_id_trimmed); // Öðrenciyi bul.
    if (!ogrenci_ptr) {
        printf("ID %s olan ogrenci bulunamadi.\n", ogrenci_id_trimmed);
        return;
    }

    printf("Iade edilecek Kitap Etiket Numarasini girin (orn., ISBN_1): > ");
    satirAl(stdin, iade_edilecek_kitap_etiketi_input, sizeof(iade_edilecek_kitap_etiketi_input));
    strncpy(iade_edilecek_kitap_etiketi_trimmed, iade_edilecek_kitap_etiketi_input, KITAP_ETIKET_UZUNLUK -1);
    iade_edilecek_kitap_etiketi_trimmed[KITAP_ETIKET_UZUNLUK-1] = '\0';
    bosluklariSil(iade_edilecek_kitap_etiketi_trimmed);

    // Ýade edilecek kopyayý bul ve öðrenciyle eþleþip eþleþmediðini kontrol et.
    mevcut_b_iter = veri->ilkKitap;
    while(mevcut_b_iter != NULL && !kopya_bulundu_ve_uygun) {
        mevcut_c_iter = mevcut_b_iter->kitapKopyalari;
        while(mevcut_c_iter != NULL && !kopya_bulundu_ve_uygun) {
            if (strcmp(mevcut_c_iter->kopyaEtiketi, iade_edilecek_kitap_etiketi_trimmed) == 0) { // Etiket eþleþti.
                if (strcmp(mevcut_c_iter->durumu, ogrenci_ptr->ogrNo) == 0) { // Kopyanýn durumu öðrenciyle eþleþti.
                    ana_kitap_ptr = mevcut_b_iter;
                    iade_edilecek_kopya_ptr = mevcut_c_iter;
                    kopya_bulundu_ve_uygun = 1;
                } else if (strcmp(mevcut_c_iter->durumu, "Rafta") == 0) { // Kopya zaten raftaysa.
                    printf("Kitap kopyasi (%s) zaten rafta. Iade islemi yapilamaz.\n", iade_edilecek_kitap_etiketi_trimmed);
                    return;
                } else { // Baþka bir öðrencide veya geçersiz durumda.
                    printf("Kitap kopyasi (%s) ogrenci %s (%s %s) uzerinde degil. Kopya durumu: %s.\n",
                           iade_edilecek_kitap_etiketi_trimmed,
                           ogrenci_ptr->ogrNo, ogrenci_ptr->adi, ogrenci_ptr->soyadi,
                           mevcut_c_iter->durumu);
                    return;
                }
            }
            if (!kopya_bulundu_ve_uygun) {
                mevcut_c_iter = mevcut_c_iter->sonrakiKopya;
            }
        }
        if (!kopya_bulundu_ve_uygun) {
            mevcut_b_iter = mevcut_b_iter->sonrakiKitap;
        }
    }

    if (!kopya_bulundu_ve_uygun) { // Kopya bulunamadý veya öðrenciyle eþleþmedi.
        printf("Etiket %s olan kitap kopyasi bulunamadi veya ogrenci %s (%s %s) uzerine kayitli degil.\n",
               iade_edilecek_kitap_etiketi_trimmed, ogrenci_ptr->ogrNo, ogrenci_ptr->adi, ogrenci_ptr->soyadi);
        return;
    }

    // Bu kopya için en son "ödünç alma" kaydýný bul (ödünç tarihini almak için).
    lr_mevcut_iter = veri->ilkOduncKaydi;
    while(lr_mevcut_iter != NULL && son_odunc_kaydi_bu_kopya_icin == NULL) {
        if(strcmp(lr_mevcut_iter->oduncKitapEtiketi, iade_edilecek_kopya_ptr->kopyaEtiketi) == 0 &&
           strcmp(lr_mevcut_iter->oduncAlanOgrNo, ogrenci_ptr->ogrNo) == 0 &&
           lr_mevcut_iter->islemTipi == 0) {
            son_odunc_kaydi_bu_kopya_icin = lr_mevcut_iter;
        }
        if (son_odunc_kaydi_bu_kopya_icin == NULL) {
            lr_mevcut_iter = lr_mevcut_iter->sonrakiOduncIslemi;
        }
    }

    if (!son_odunc_kaydi_bu_kopya_icin) { // Ödünç alma kaydý bulunamadýysa (normalde olmamalý).
        printf("Hata: Kitap %s icin ogrenci %s tarafindan eslesen aktif bir odunc alma kaydi bulunamadi. Iade islemi yapiliyor ancak gecikme hesaplanamayacak.\n",
               iade_edilecek_kopya_ptr->kopyaEtiketi, ogrenci_ptr->ogrNo);
    } else {
        strcpy(odunc_tarihi_str_buffer, son_odunc_kaydi_bu_kopya_icin->islemTarihi); // Ödünç tarihini al.
    }

    // Kopyanýn durumunu "Rafta" olarak güncelle.
    strcpy(iade_edilecek_kopya_ptr->durumu, "Rafta");
    bugununTarihiniAl(guncel_tarih_str_buffer); // Bugünün tarihini al.

    // Gecikme kontrolü ve ceza puaný.
    if (strlen(odunc_tarihi_str_buffer) > 0) { // Eðer ödünç tarihi bulunduysa.
        gun_farki_val = gunFarkiniHesapla(odunc_tarihi_str_buffer, guncel_tarih_str_buffer); // Gün farkýný hesapla.
        if (gun_farki_val == -9999) {
             printf("Uyari: Odunc veya guncel tarih gecersiz, gun farki hesaplanamadi. Ceza uygulanmiyor.\n");
        } else {
            printf("Kitap %s tarihinde odunc alinmisti, %s tarihinde iade edildi. Odunc suresi: %d gun.\n",
                   odunc_tarihi_str_buffer, guncel_tarih_str_buffer, gun_farki_val);
            if (gun_farki_val > MAX_ODUNC_SURESI) { // Süre aþýldýysa (15 günü geçtiyse).
                int ceza_miktari = 10; // PDF'e göre sabit 10 puan ceza.
                ogrenci_ptr->puan -= ceza_miktari; // Öðrencinin puanýný düþür.
                printf("Gec iade! Kitap %d gunden sonra iade edildigi icin %d puan ceza uygulandi. Ogrenci %s (%s %s) yeni puani: %d.\n",
                       MAX_ODUNC_SURESI, ceza_miktari,
                       ogrenci_ptr->ogrNo, ogrenci_ptr->adi, ogrenci_ptr->soyadi, ogrenci_ptr->puan);
                ogrencileriDosyayaYaz(veri); // Öðrenci puanýný dosyaya yaz.
            }
        }
    }

    // Ýade iþlemini kaydet (iþlem tipi 1: Ýade Etme).
    iade_kaydi_ptr = yeniOduncKaydiOlustur(iade_edilecek_kopya_ptr->kopyaEtiketi, ogrenci_ptr->ogrNo, 1, guncel_tarih_str_buffer);
    if (iade_kaydi_ptr) { // Kayýt oluþturma baþarýlýysa.
        oduncKaydiniListeyeEkle(veri, iade_kaydi_ptr); // Kaydý listeye ekle.
        kitapKopyalariniDosyayaYaz(veri); // Kopyanýn yeni durumunu dosyaya yaz.
        oduncKayitlariniDosyayaYaz(veri); // Ýade kaydýný dosyaya yaz.
        printf("Kitap '%s' (Kopya: %s) ogrenci %s (%s %s) tarafindan %s tarihinde basariyla iade edildi.\n",
               ana_kitap_ptr->kitapAdi, iade_edilecek_kopya_ptr->kopyaEtiketi,
               ogrenci_ptr->ogrNo, ogrenci_ptr->adi, ogrenci_ptr->soyadi, guncel_tarih_str_buffer);
    } else { // Kayýt oluþturma baþarýsýzsa.
        // Kopyanýn durumunu geri öðrenciye ata (iade iþlemi baþarýsýz olduðu için).
        strncpy(iade_edilecek_kopya_ptr->durumu, ogrenci_ptr->ogrNo, OGRENCI_NO_UZUNLUK -1);
        iade_edilecek_kopya_ptr->durumu[OGRENCI_NO_UZUNLUK-1] = '\0';
        printf("HATA: Iade kaydi olusturulamadi. Kitap iade islemi tamamlanamadi, kopya durumu geri alindi.\n");
    }
}

// Öðrenci iþlemleri menüsünü gösteriyor ve kullanýcýnýn seçimine göre ilgili fonksiyonu çaðýrýyor.
void ogrenciIslemleriMenusu(KutuphaneVerisi* veri) {
    int secim;
    do {
        printf("\n--- Ogrenci Islemleri Menusu ---\n");
        printf("1. Ogrenci Ekle\n");
        printf("2. Ogrenci Sil\n");
        printf("3. Ogrenci Bilgilerini Guncelle\n");
        printf("4. Ogrenci Bilgilerini ve Odunc Gecmisini Goruntule\n");
        printf("5. Kitabi Iade Etmemis Ogrencileri Listele\n");
        printf("6. Cezali Ogrencileri Listele (Puan < 0)\n");
        printf("7. Tum Ogrencileri Listele\n");
        printf("0. Ana Menuye Don\n");
        printf("Seciminizi girin: > ");
        if (scanf("%d", &secim) != 1) { // Geçerli sayý girilmezse.
            secim = -1;
        }
        while(getchar()!='\n'); // Buffer'ý temizle.

        switch (secim) {
            case 1: ogrenciEkleArayuz(veri); break;
            case 2: ogrenciSilArayuz(veri); break;
            case 3: ogrenciDuzenleArayuz(veri); break;
            case 4: ogrenciDetaylariniGosterArayuz(veri); break;
            case 5: iadeEtmeyenOgrencileriGoster(veri); break;
            case 6: cezaliOgrencileriGoster(veri); break;
            case 7: butunOgrencileriGoster(veri); break;
            case 0: printf("Ana Menuye donuluyor...\n"); break;
            default: printf("Gecersiz secim. Lutfen tekrar deneyin.\n");
        }
    } while (secim != 0); // Kullanýcý 0 girene kadar döngü devam eder.
}

// Kitap iþlemleri menüsünü gösteriyor.
void kitapIslemleriMenusu(KutuphaneVerisi* veri) {
    int secim;
    do {
        printf("\n--- Kitap Islemleri Menusu ---\n");
        printf("1. Kitap Ekle\n");
        printf("2. Kitap Sil\n");
        printf("3. Kitap Bilgilerini Guncelle (Sadece Ad - Dikkat: Siralama Bozulabilir!)\n");
        printf("4. Kitap Bilgilerini ve Kopyalarini Goruntule\n");
        printf("5. Raftaki Kitaplari Listele\n");
        printf("6. Zamaninda Teslim Edilmeyen Kitaplari Listele\n");
        printf("7. Kitabi Yazara Ata\n");
        printf("8. Kitabin Yazar(lar)ini Guncelle (Ekle/Cikar)\n");
        printf("0. Ana Menuye Don\n");
        printf("Seciminizi girin: > ");
        if (scanf("%d", &secim) != 1) {
            secim = -1;
        }
        while(getchar()!='\n');

        switch (secim) {
            case 1: kitapEkleArayuz(veri); break;
            case 2: kitapSilArayuz(veri); break;
            case 3: kitapDuzenleArayuz(veri); break;
            case 4: butunKitapVeKopyalariGoster(veri); break;
            case 5: raftakiKitaplariGoster(veri); break;
            case 6: gecikenKitaplariGoster(veri); break;
            case 7: kitabaYazarAtaArayuz(veri); break;
            case 8: kitapYazarGuncelleArayuz(veri); break;
            case 0: printf("Ana Menuye donuluyor...\n"); break;
            default: printf("Gecersiz secim. Lutfen tekrar deneyin.\n");
        }
    } while (secim != 0);
}

// Yazar iþlemleri menüsünü gösteriyor.
void yazarIslemleriMenusu(KutuphaneVerisi* veri) {
    int secim;
    do {
        printf("\n--- Yazar Islemleri Menusu ---\n");
        printf("1. Yazar Ekle\n");
        printf("2. Yazar Sil\n");
        printf("3. Yazar Bilgilerini Guncelle\n");
        printf("4. Yazar Bilgilerini ve Kitaplarini Goruntule\n");
        printf("5. Tum Yazarlari Listele\n");
        printf("0. Ana Menuye Don\n");
        printf("Seciminizi girin: > ");
        if (scanf("%d", &secim) != 1) {
            secim = -1;
        }
        while(getchar()!='\n');

        switch (secim) {
            case 1: yazarEkleArayuz(veri); break;
            case 2: yazarSilArayuz(veri); break;
            case 3: yazarDuzenleArayuz(veri); break;
            case 4: yazarinKitaplariniGosterArayuz(veri); break;
            case 5: butunYazarlariGoster(veri); break;
            case 0: printf("Ana Menuye donuluyor...\n"); break;
            default: printf("Gecersiz secim. Lutfen tekrar deneyin.\n");
        }
    } while (secim != 0);
}

// Ana menüyü gösteriyor ve programýn ana döngüsünü oluþturuyor.
void anaMenuGoster(KutuphaneVerisi* veri) {
    int secim;
    do {
        printf("\n<<< KUTUPHANE OTOMASYONU ANA MENU >>>\n");
        printf("    -----------------------------\n");
        printf("1. Ogrenci Islemleri\n");
        printf("2. Kitap Islemleri\n");
        printf("3. Yazar Islemleri\n");
        printf("4. Kitap Odunc Al\n");
        printf("5. Kitap Iade Et\n");
        printf("0. Programdan Cik\n");
        printf("Seciminizi girin: > ");
        if (scanf("%d", &secim) != 1) {
            secim = -1;
        }
        while(getchar()!='\n');

        switch (secim) {
            case 1: ogrenciIslemleriMenusu(veri); break;
            case 2: kitapIslemleriMenusu(veri); break;
            case 3: yazarIslemleriMenusu(veri); break;
            case 4: kitapOduncAlArayuz(veri); break;
            case 5: kitapIadeEtArayuz(veri); break;
            case 0: printf("Kutuphane Sisteminden cikiliyor...\n"); break;
            default: printf("Gecersiz secim. Lutfen tekrar deneyin.\n");
        }
    } while (secim != 0); // Kullanýcý 0 girene kadar program çalýþmaya devam eder.
}

