# CtrLang 🌐

> **Türkçe backend web dili** — Saf C, SQLite, her platformda.

[![Lisans: GPL v3](https://img.shields.io/badge/Lisans-GPLv3-blue.svg)](LICENSE)
[![C11](https://img.shields.io/badge/C-C11-blue.svg)]()
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS%20%7C%20Windows%20%7C%20Android-green.svg)]()

---

## 🎯 Nedir?

CtrLang, **Türkçe anahtar kelimelerle** yazılan bir **backend web dilidir**.

```ctrlang
sunucu başlat(3000)

sayfa "/" {
    yazdır "Merhaba CtrLang!"
}
```

**Çalıştır:**

```bash
ctr --calistir merhaba.ctr
```

Tarayıcıda: `http://localhost:3000`

---

## ✨ Özellikler

- 🇹🇷 **Türkçe sözdizimi** — `yazdır`, `eğer`, `döngü`, `işlev`
- ⚡ **Saf C** — Hızlı, hafif (~2 MB bellek)
- 🗄️ **SQLite** — Yerleşik veritabanı
- 🌐 **HTTP sunucu** — Mongoose ile
- 🎨 **HTML üretimi** — `html { }` bloğu
- 📦 **JSON API** — `json { }` bloğu
- 🛣️ **Dinamik rotalar** — `/kullanici/{id}`
- 🌍 **Çapraz platform** — Linux, macOS, Windows, Android

---

## 📦 Kurulum

### Linux / macOS / Termux

```bash
git clone https://github.com/darking053official/CtrLang.git
cd CtrLang
chmod +x kur.sh
./kur.sh
```

### Windows (CMD)

```cmd
git clone https://github.com/darking053official/CtrLang.git
cd CtrLang
kur.bat
```

### Windows (PowerShell)

```powershell
git clone https://github.com/darking053official/CtrLang.git
cd CtrLang
.\kur.ps1
```

---

## 📖 Kullanım

```bash
ctr --platform              # Platform bilgisi
ctr --token merhaba.ctr     # Token'ları gör
ctr --ast merhaba.ctr       # AST'yi gör
ctr merhaba.ctr             # C kodu üret
ctr --derle merhaba.ctr     # C'ye çevir ve derle
ctr --calistir merhaba.ctr  # Derle ve çalıştır
```

---

## 🔤 Dil Referansı

### Değişkenler

```ctrlang
sayı x = 10
metin isim = "Ali"
mantık aktif = doğru
```

### Kontrol

```ctrlang
eğer x > 5 {
    yazdır "Büyük"
} değilse {
    yazdır "Küçük"
}

döngü i = 0, i < 10, i = i + 1 {
    yazdır i
}

iken x < 100 {
    x = x + 1
}
```

### İşlevler

```ctrlang
işlev topla(a, b) {
    döndür a + b
}

yazdır topla(3, 5)
```

### Web

```ctrlang
sunucu başlat(3000)

sayfa "/" {
    yazdır "Ana sayfa"
}

sayfa "/kullanici/{id}" {
    yazdır "Kullanıcı: " + id
}
```

### HTML

```ctrlang
sayfa "/" {
    html {
        başlık "Hoş geldin"
        paragraf "CtrLang ile yazıldı"
        düğme "Tıkla"
    }
}
```

### Veritabanı

```ctrlang
veritabanı bağlan("blog.db")
veritabanı.çalıştır("CREATE TABLE IF NOT EXISTS yazilar (id INTEGER PRIMARY KEY, baslik TEXT)")
liste yazilar = veritabanı.sorgu("SELECT * FROM yazilar")
```

---

## 🏗️ Mimari

```
.ctr → Lexer → Parser → AST → C kodu → gcc → Sunucu
```

---

## 📁 Proje Yapısı

```
CtrLang/
├── src/                    # Derleyici
├── kutuphane/              # Runtime
├── vendor/                 # Harici kütüphaneler
├── ornekler/               # .ctr örnekleri
├── kur.sh                  # Linux/macOS/Termux
├── kur.bat                 # Windows CMD
├── kur.ps1                 # Windows PowerShell
└── Makefile
```

---

## 📋 Yol Haritası

- [x] Lexer, Parser, AST
- [x] C kodu üretici
- [x] HTTP sunucu (Mongoose)
- [x] HTML + JSON
- [x] SQLite
- [x] Çapraz platform
- [ ] Standart kütüphane
- [ ] Şablon motoru
- [ ] Oturum yönetimi
- [ ] v1.0

---

## 📜 Lisans

**GNU General Public License v3.0** — [LICENSE](LICENSE)

---

**⭐ Beğendiyseniz yıldız verin!**

🇹🇷 **Türkçe programlama için bir adım.**
