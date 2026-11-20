# re3 Enhanced — roadmap

Güncelleme 20 Kasım 2026. Son paket **v1.27 OpenGL**. Son kullanıcı
geri bildirimlerine ait değişiklikleri kullanıcı v1.27 ile oyun içinde onayladı.

## Paketteki özellikler ve kalan doğrulamalar

### Uzak araç ve yaya gölgeleri

Graphics menüsünde **Distant Vehicle and Pedestrian Shadows**, **Distant
and Crowded World** seçeneğinin altında ve varsayılan olarak kapalı.
İkisi de açıkken araç ve yaya gölgeleri mevcut görünürlük sınırlarına kadar
çizilir. Menüde yerleşim, gölge mesafesi ve yoğun trafikte performans için
derleme ve oyun içi doğrulama bekliyor.

### RM-01 — Liberty City Trainer

Mevcut trainer komutları ve ayarları **F8** ile açılan tek liste panelinde.
Referans görseldeki kategori yolu, kaydırılabilir liste, seçili satır, sıra
sayacı ve tuş yardımı GTA III renkleriyle yeniden tasarlandı. Enter alt
menüyü açar veya komutu çalıştırır, Esc veya Backspace bir üst kategoriye döner ve ana listede trainer’ı kapatır.
F8 herhangi bir seviyeden kapatır.
Trainer klavye ile kullanılır. Açıkken oyun hareketi ve fare kamerası çalışır. Tuş tekrarı oyun hızından bağımsızdır.

Doğrulama: farklı FPS sınırlarında gezinme, uzun araç listeleri, trainer açıkken
NPC ve trafik hareketi, oluşturulan araçların sesi ve kapanışta oyun kontrolleri.

### RM-02 — Radyo çarkı

Gameplay Setup içindeki **Radio Wheel** açıkken araçta **R** basılı
tutulur. Fare, oklar veya scroll ile seçim yapılır; R bırakılınca istasyona
geçilir. R basılı değilken scroll klasik istasyon göstergesiyle çalışır.
İstasyon adları ortak oyun metninden alınır, yerleşik logolar korunur.
MP3 yuvası parça mevcutsa gösterilir. Radio Off en alttadır. Çarktan farklı
bir istasyon seçilince klasik radyo geçiş süresi ve sesleri kullanılır.

Doğrulama: istasyon değiştirme, radyoyu kapatıp tekrar açma, MP3 durumu,
görev radyosu ve polis/telsizli araçlar. Kontrolcü erişimi sonraki iterasyonda.

### RM-03 — Silah çarkı

Gameplay Setup içindeki **Weapon Wheel** açıkken yaya olarak **Tab**
basılı tutulur. GTA III HUD ikonları sabit silah yuvalarında gösterilir.
Sahip olunmayan silahların yuvası boş kalır. İsim merkezde, HUD ile aynı
biçimdeki mühimmat bağımsız alt satırdadır. Unarmed en alttadır. Tab
bırakılınca seçili silah kuşanılır. Oyun hızı ve oyun sesi perdesi çark
açıkken düşer, kapanışta geri döner. İki çark da dik yazı tipi kullanır
ve uzun adları merkezdeki boşluğa sığdırır. Alt kontrol satırı kaldırıldı.

Doğrulama: az ve dolu envanter, boş mühimmat, duraklatma, Alt+Tab,
silah kuşanma ve zaman akışının geri dönmesi. Kontrolcü erişimi sonraki iterasyonda.

### RM-06 — Skip Intros

Windows'ta **Options → Gameplay Setup → Skip Intros** varsayılan OFF. OFF iken
oyun kurulumundaki Rockstar ve GTA açılış videoları oynatılmaya çalışılır.
ON iken açılış videoları atlanır. Tercih `[General] NoMovies` olarak
saklanır. Windows OpenGL/GLFW oynatımı sistemin DirectShow desteğini kullanır,
video eksikse veya oynatılamazsa açılış devam eder. Video oynatılırken
Windows imleci gizlenir. Görev sinematikleri
normal oyun akışını izler. Windows dışı OpenGL açılışında video oynatma desteği bekliyor.

Doğrulama: temiz INI ile açılış, ON/OFF seçiminin yeniden açılışta
uygulanması, video bulunamaması ve farklı ekran biçimlerinde açılış.

## v1.27 ile doğrulanan kayıt değişiklikleri

Kullanıcı önceki hata kuyruğunu diğer sohbetlerde tamamladığını bildirdi.
Geçmiş kayıtlar [BUGS.md](docs/BUGS.md) içinde.

- **SAV-07** · Autosave veya F5 yüklemesinde devam eden eski betiklerin sol üstteki
  yardım mesajları, evden ayrıldıktan sonra da tekrarlanmamalı. Yeni
  oynanışın bildirimleri ve canlı görev sonu akışı korunmalı.
- **SAV-08** · Ev çıkışındaki karakter ve kamera, özgün ev kaydı yönünü
  kullanmalı.
- **SAV-09** · Autosave yalnızca Load Game ekranında, sekiz manuel slottan
  sonra ayrı bölümde yer almalı.

Kullanıcı v1.27 ile bu kayıt davranışlarının beklentisine uyduğunu bildirdi.

## Sonraki paket öncesi

- Temiz GTA III kurulumunda OpenGL x64 başlangıcı, ekran biçimleri ve kayıt
  yükleme. v1.26 doğrulanan Fullscreen/Borderless yakalama ve imleç davranışları
  sonraki paket için regresyon kontrolü.
- ASI Loader günlüğünde x64 yüklemesini, CLEO Redux günlüğünde re3 sembol
  çözümlemesini ve küçük bir JS/TS betiğinin çalışmasını doğrula.
- Eşleşen EXE/PDB ve oyun runtime kitaplıklarıyla SHA-256 manifestli tek
  sürüm klasörü ve ZIP. Kurulum notları ve ASI/CLEO sınırları
  [ENHANCED.md](ENHANCED.md) içinde.

## Çalışma biçimi

Kullanıcı **“build al”** dediğinde biriken kaynak değişiklikleri toplu
doğrulanır. Derleme, CI birim kontrolleri ve oyun içi sonuçlar ayrı izlenir.
Sonraki tam paket sürümlenir ve önceki paketler korunur.
