<div align="center">

<img src="resimler/deepseek.png" width="120" alt="DeepSeek">

# 🌐 CtrLang

### Türkçe Backend Web Dili

**Saf C ile yazıldı. SQLite ile güçlendirildi. Her platformda çalışır.**

<br>

[![Lisans](https://img.shields.io/badge/Lisans-GPLv3-blue.svg)](LICENSE)
[![C11](https://img.shields.io/badge/C-C11-blue.svg)]()
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS%20%7C%20Windows%20%7C%20Android-green.svg)]()
[![Sürüm](https://img.shields.io/badge/Sürüm-0.2.0-orange.svg)]()

<br>

<a href="#-nedir">Nedir?</a> ·
<a href="#-özellikler">Özellikler</a> ·
<a href="#-kurulum">Kurulum</a> ·
<a href="#-kullanım">Kullanım</a> ·
<a href="#-dil-referansı">Dil</a>

</div>

---

<h2 id="-nedir">🎯 Nedir?</h2>

<p><b>CtrLang</b>, Türkçe anahtar kelimelerle yazılan bir <b>backend web dilidir</b>. PHP ve Node.js'in Türkçe alternatifi olmayı hedefler.</p>

<pre><code>sunucu başlat(3000)

sayfa "/" {
    yazdır "Merhaba CtrLang!"
}</code></pre>

<p><b>Çalıştır:</b></p>

<pre><code>ctr run merhaba.ctr</code></pre>

<p>Tarayıcıda: <code>http://localhost:3000</code></p>

---

<h2 id="-özellikler">✨ Özellikler</h2>

<table>
<tr>
<td>🇹🇷 <b>Türkçe sözdizimi</b></td>
<td><code>yazdır</code>, <code>eğer</code>, <code>döngü</code>, <code>işlev</code></td>
</tr>
<tr>
<td>⚡ <b>Saf C</b></td>
<td>Hızlı, hafif (~2 MB bellek)</td>
</tr>
<tr>
<td>🗄️ <b>SQLite</b></td>
<td>Yerleşik veritabanı</td>
</tr>
<tr>
<td>🌐 <b>HTTP sunucu</b></td>
<td>Mongoose ile</td>
</tr>
<tr>
<td>🎨 <b>HTML + JSON</b></td>
<td>Yerleşik üretim</td>
</tr>
<tr>
<td>🛣️ <b>Dinamik rotalar</b></td>
<td><code>/kullanici/{id}</code></td>
</tr>
<tr>
<td>🌍 <b>Çapraz platform</b></td>
<td>Linux, macOS, Windows, Android</td>
</tr>
</table>

---

<h2 id="-kurulum">📦 Kurulum</h2>

<h3>🐧 Linux / 🍎 macOS / 🤖 Termux</h3>

<pre><code>git clone https://github.com/darking053official/CtrLang.git
cd CtrLang
./kur.sh</code></pre>

<h3>🪟 Windows</h3>

<pre><code>git clone https://github.com/darking053official/CtrLang.git
cd CtrLang
kur.bat</code></pre>

---

<h2 id="-kullanım">📖 Kullanım</h2>

<pre><code>ctr new blogum           # Yeni proje
ctr run app.ctr          # Derle ve çalıştır
ctr run app.ctr -p 8080  # Port belirt
ctr run app.ctr -h 0.0.0.0
ctr listele              # Tüm komutlar
ctr update               # Güncelle</code></pre>

---

<h2 id="-dil-referansı">🔤 Dil Referansı</h2>

<h3>Değişkenler</h3>

<pre><code>sayı x = 10
metin isim = "Ali"
mantık aktif = doğru
liste yazilar = veritabanı.sorgu("SELECT * FROM yazilar")</code></pre>

<h3>Kontrol</h3>

<pre><code>eğer x > 5 {
    yazdır "Büyük"
} değilse {
    yazdır "Küçük"
}

döngü i = 0, i < 10, i = i + 1 {
    yazdır i
}</code></pre>

<h3>İşlevler</h3>

<pre><code>işlev topla(a, b) {
    döndür a + b
}</code></pre>

<h3>Web</h3>

<pre><code>sunucu başlat(3000)

sayfa "/" {
    yazdır "Ana sayfa"
}

sayfa "/kullanici/{id}" {
    yazdır "Kullanıcı: " + id
}</code></pre>

<h3>HTML</h3>

<pre><code>html {
    başlık "Hoş geldin"
    paragraf "CtrLang ile yazıldı"
    düğme "Tıkla"
}</code></pre>

<h3>Veritabanı</h3>

<pre><code>veritabanı bağlan("blog.db")
veritabanı.çalıştır("CREATE TABLE IF NOT EXISTS yazilar (id INTEGER PRIMARY KEY)")
liste yazilar = veritabanı.sorgu("SELECT * FROM yazilar")</code></pre>

---

<h2>📋 Yol Haritası</h2>

<ul>
<li>✅ Lexer, Parser, AST</li>
<li>✅ C kodu üretici</li>
<li>✅ HTTP sunucu + HTML + JSON</li>
<li>✅ SQLite + dinamik rotalar</li>
<li>✅ Çapraz platform</li>
<li>⬜ Standart kütüphane</li>
<li>⬜ Şablon motoru</li>
<li>⬜ WebSocket</li>
<li>⬜ v1.0</li>
</ul>

---

<h2>📜 Lisans</h2>

<p><b>GNU General Public License v3.0</b> — <a href="LICENSE">LICENSE</a></p>

<table>
<tr><th>Kütüphane</th><th>Lisans</th></tr>
<tr><td>Mongoose</td><td>GPL v2</td></tr>
<tr><td>SQLite</td><td>Public Domain</td></tr>
<tr><td>cJSON</td><td>MIT</td></tr>
<tr><td>sds</td><td>BSD</td></tr>
<tr><td>uthash</td><td>BSD</td></tr>
</table>

---

<h2>👥 Katkıda Bulunanlar</h2>

<ul>
<li><b>darking053official</b> — Proje sahibi</li>
<li><b>Deepseek</b> — AI asistan, kod geliştirme</li>
</ul>

---

<div align="center">

<h3>⭐ Beğendiyseniz yıldız verin!</h3>

<p>🇹🇷 <b>Türkçe programlama için bir adım.</b></p>

</div>
