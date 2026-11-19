# re3 Enhanced — roadmap

Güncelleme 19 Kasım 2026. Son paket **v1.18 OpenGL**. Bu sayfadaki yeni
özellikler derlendi. Oyun içi doğrulama bekliyor.

## Geliştirmedeki özellikler

### RM-01 — Liberty City Trainer

Mevcut trainer komutları ve ayarları **F8** ile açılan tek liste panelinde.
Referans görseldeki kategori yolu, kaydırılabilir liste, seçili satır, sıra
sayacı ve tuş yardımı GTA III renkleriyle yeniden tasarlandı. Enter alt
menüyü açar veya komutu çalıştırır, Backspace bir üst kategoriye döner.
Oklar ve fare ile gezinilebilir. Tuş tekrar süresi oyun hızından bağımsız
saatle ölçülür ve giriş oyun güncellemesinde işlenir.

Doğrulama: farklı FPS sınırlarında gezinme ve ayar değişimi, fareyle seçim,
uzun araç listeleri ve menü kapanınca oyun kontrollerinin geri gelmesi.

### RM-02 — Radyo çarkı

Radyo bulunan araçta **F6** basılı tutulur. Fare yönü veya sol/sağ ok
tuşlarıyla istasyon seçilir. F6 bırakılınca istasyona geçilir, Backspace
seçimi iptal eder. Audio menüsündeki yerleşik istasyon logoları kullanılır.
MP3 yuvası parça mevcutsa gösterilir. Çarkta radyo kapatma seçeneği bulunur.

Doğrulama: istasyon değiştirme, radyoyu kapatıp tekrar açma, MP3 durumu,
görev radyosu ve polis/telsizli araçlar. Kontrolcü erişimi sonraki iterasyonda.

### RM-03 — Silah çarkı

Radyo çarkıyla aynı seçim altyapısı. Yaya olarak **F7** basılı tutulur.
GTA III HUD ikonları, oyuncunun kullanabildiği silahlar ve seçilen silahın
şarjör/toplam mühimmatı gösterilir. Silahı donatmak için F7 bırakılır.
Backspace iptal eder. Çark açıkken oyun hızı en fazla %25'tir ve kapanışta
normal zaman akışı geri gelir. Yerleşim için Definitive Edition'daki
kategori çarkı, yavaşlatma davranışı için GTA V referans alındı.

Doğrulama: az ve dolu envanter, boş mühimmat, iptal, duraklatma, Alt+Tab,
silah kuşanma ve zaman akışının geri dönmesi. Kontrolcü erişimi sonraki iterasyonda.

### RM-06 — Skip Intros

Windows'ta **Options → Gameplay → Skip Intros** varsayılan OFF. OFF iken
oyun kurulumundaki Rockstar ve GTA açılış videoları oynatılmaya çalışılır.
ON iken açılış videoları atlanır. Tercih `[General] NoMovies` olarak
saklanır. Windows OpenGL/GLFW oynatımı sistemin DirectShow desteğini kullanır,
video eksikse veya oynatılamazsa açılış devam eder. Görev sinematikleri
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
