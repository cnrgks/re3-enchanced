# re3 Enhanced — roadmap

Güncelleme 20 Kasım 2026. Son paket **v1.19 OpenGL**. Bu sayfadaki son
v1.18 geri bildirimlerine ait değişiklikler derlendi. Oyun içi doğrulama bekliyor.

## Geliştirmedeki özellikler

### RM-01 — Liberty City Trainer

Mevcut trainer komutları ve ayarları **F8** ile açılan tek liste panelinde.
Referans görseldeki kategori yolu, kaydırılabilir liste, seçili satır, sıra
sayacı ve tuş yardımı GTA III renkleriyle yeniden tasarlandı. Enter alt
menüyü açar veya komutu çalıştırır, Esc veya Backspace bir üst kategoriye döner ve ana listede trainer’ı kapatır.
F8 herhangi bir seviyeden kapatır.
Oklar ve fare ile gezinilebilir. Tuş tekrar süresi oyun hızından bağımsız
saatle ölçülür ve giriş oyun güncellemesinde işlenir.

Doğrulama: farklı FPS sınırlarında gezinme ve ayar değişimi, fareyle seçim,
uzun araç listeleri ve menü kapanınca oyun kontrollerinin geri gelmesi.

### RM-02 — Radyo çarkı

Gameplay Setup içindeki **Radio Wheel** açıkken araçta **R** basılı
tutulur. Fare, oklar veya scroll ile seçim yapılır; R bırakılınca istasyona
geçilir. R basılı değilken scroll klasik istasyon göstergesiyle çalışır.
İstasyon adları ortak oyun metninden alınır, yerleşik logolar korunur.
MP3 yuvası parça mevcutsa gösterilir. Radio Off en alttadır.

Doğrulama: istasyon değiştirme, radyoyu kapatıp tekrar açma, MP3 durumu,
görev radyosu ve polis/telsizli araçlar. Kontrolcü erişimi sonraki iterasyonda.

### RM-03 — Silah çarkı

Gameplay Setup içindeki **Weapon Wheel** açıkken yaya olarak **Tab**
basılı tutulur. GTA III HUD ikonları, kullanılabilir silahlar ve seçilen
silahın şarjör/toplam mühimmatı gösterilir. Unarmed en alttadır. Tab
bırakılınca seçili silah kuşanılır. Oyun hızı ve oyun sesi perdesi çark
açıkken düşer, kapanışta geri döner. İki çark da dik yazı tipi kullanır
ve uzun adları merkezdeki boşluğa sığdırır. Alt kontrol satırı kaldırıldı.

Doğrulama: az ve dolu envanter, boş mühimmat, duraklatma, Alt+Tab,
silah kuşanma ve zaman akışının geri dönmesi. Kontrolcü erişimi sonraki iterasyonda.

### RM-06 — Skip Intros

Windows'ta **Options → Gameplay → Skip Intros** varsayılan OFF. OFF iken
oyun kurulumundaki Rockstar ve GTA açılış videoları oynatılmaya çalışılır.
ON iken açılış videoları atlanır. Tercih `[General] NoMovies` olarak
saklanır. Windows OpenGL/GLFW oynatımı sistemin DirectShow desteğini kullanır,
video eksikse veya oynatılamazsa açılış devam eder. Video oynatılırken
Windows imleci gizlenir. Görev sinematikleri
normal oyun akışını izler. Windows dışı OpenGL açılışında video oynatma desteği bekliyor.

Doğrulama: temiz INI ile açılış, ON/OFF seçiminin yeniden açılışta
uygulanması, video bulunamaması ve farklı ekran biçimlerinde açılış.

## Hata ve doğrulama kuyruğu

- **SAV-01–06** · F5 konumu ve kısıtları, üç ada ev noktası, görev sonrası
  autosave/yükleme ve HUD bildirimi.
- **DIST-01/02** · Trafik ve yaya üretimi, yeni dünya ayarı, park edilmiş araçlar
  ve fiziksel havuz sınırları.
- **INPUT-04/05, WIN-02** · Tek basışta araca giriş, Alt+Tab imleci ve odak
  dönüşünde Pause menüsü.
- **MAP-01–07, GPS-04, UI-03** · Harita, rota ve Restore Defaults kapsamındaki
  açık doğrulamalar.
- **Options** · Yeni menü sırası ve Gameplay/Graphics/Display düzeni.

Kayıt bazındaki durumlar, kabul ölçütleri ve geçmiş sonuçlar [BUGS.md](docs/BUGS.md)
içinde. OpenGL ana geliştirme yolu, D3D9 alternatif. Bu karar
[mimari notlarında](docs/ARCHITECTURE.md) kayıtlı.

## Çalışma biçimi

Kullanıcı **“build al”** dediğinde biriken kaynak değişiklikleri toplu
doğrulanır. Derleme, CI birim kontrolleri ve oyun içi sonuçlar ayrı izlenir.
Sonraki tam paket sürümlenir ve önceki paketler korunur.
