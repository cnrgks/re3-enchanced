# v1.1 kullanıcı geri bildirimi — hata ve davranış kayıtları

Tarih: 19 Kasım 2026. Kaynak: [özgün notlar](feedback/2026-11-19/original-notes.txt)
ve aşağıdaki beş ekran görüntüsü. Bu dosya mevcut yayımlanmış v1.1'in
geri bildirimini kaydeder; burada listelenen maddeler henüz düzeltilmiş sayılmaz.

## Çalışma ve doğrulama kuralı

- Önce kayıtlar tutulur, ardından maddeler tek tek ele alınır.
- Kullanıcı açıkça **“build al”** demeden derleme, test paketi, workflow dispatch
  veya otomatik build başlatan push yapılmaz. Her değişiklikte yeniden build alınmaz.
- Durumlar: **açık**, **inceleme gerekiyor**, **kaynakta değiştirildi / derlenmedi**,
  **derlendi / oyun testi bekliyor**, **kullanıcı doğruladı**. Bunlar birbirinin yerine kullanılmaz.
- Bir maddenin kapanması için aşağıdaki beklenen davranış sağlanmalıdır.
- v1.1 paketi ve eski sürümler değiştirilmez. Sonraki tam paket kullanıcı istediğinde sürümlenir.

## Ekran görüntüsü dizini

| Kanıt | Görüntüde görülen / notla ilişkilendirilen durum |
| --- | --- |
| [SS1 — çift ev](feedback/2026-11-19/01-double-safehouse.png) | Aynı ev bölgesinde iki mor ev simgesi. Beyaz oyuncu işareti ek bir hata olarak yorumlanmadı. |
| [SS2 — üçüncü ada](feedback/2026-11-19/02-undiscovered-island.png) | Üçüncü adada servis/ev ikonları. Adanın henüz açılmadığı bilgisi kullanıcının notundan geliyor. |
| [SS3 — alt şerit ve rota](feedback/2026-11-19/03-map-footer-and-route.png) | Sarı MAP şeridi hâlâ var, Back soldan kesiliyor; kırmızı rota hedef işaretinden önce sonlanıyor gibi görünüyor. |
| [SS4 — rota](feedback/2026-11-19/04-route-gap-and-detour.png) | U biçimli geri dönüş/çift hat ve hedefe bağlantısı belirsiz rota. Yol katmanı ve bağlantı sürekliliği incelenecek. |
| [SS5 — kayıt koşulları](feedback/2026-11-19/05-save-restrictions.png) | Örnek modun dört koşulu: görevde değil, araçta değil, hareketsiz, aranma seviyesi yok. |

## Kayıtlar

| Kimlik | Konu | Durum |
| --- | --- | --- |
| SAV-01 | F5 kaydı kendi konumunu korumalı | Açık — davranış düzeltmesi |
| SAV-02 | Görev sonrası evde çatıda doğma | Açık — kullanıcı bildirdi |
| SAV-03 | Quicksave engelleme koşulları | İstenen davranış / kontrol edilecek |
| VEH-01 | Kırık ön camdan sürücü hasar almıyor | Açık — kullanıcı bildirdi |
| DIST-01 | Vehicle Distance 1× üzerindeyken araç spawn akışı bozuluyor | Açık — çarpanlar ve araç adet sınırı incelenecek |
| DIST-02 | Pedestrian Distance 1× üzerindeyken yaya spawn akışı bozuluyor | Açık — çarpanlar ve yaya adet sınırı incelenecek |
| GPS-01 | Elle waypoint'te üst/alt yol katmanı karışıyor | Açık — kullanıcı bildirdi |
| GPS-02 | Rota boşlukları ve anlamsız geri dönüşler | Açık — SS3/SS4 |
| MAP-01 | Çift ev ikonu | Açık — SS1/SS3 |
| MAP-02 | Keşfedilmemiş/açılmamış adadaki ikonlar | Açık — SS2 ve not |
| MAP-03 | Daha önce görülen payphone ikonu eksik | Açık — kullanıcı bildirdi |
| MAP-04 | Sarı MAP şeridinin tamamı kaldırılmalı | Açık — önceki istek eksik uygulanmış |
| MAP-05 | Back kesiliyor / hizası bozuk | Açık — SS3 |
| INPUT-01 | OpenGL–D3D9 fare hassasiyeti farklı | Açık — kullanıcı bildirdi |
| CAP-01 | OpenGL fullscreen/borderless yakalama | v1.5 Discord paylaşımı başarılı. v1.6 açılış ve Alt-Tab sırasında tüm monitör kararıyor. Ambilight açık |
| INPUT-02 | Borderless kayıtlıyken yeniden açılışta imleç sola kilitleniyor | v1.4 kaynak düzeltmesi derlendi, oyun içi doğrulama bekliyor |
| INPUT-03 | Borderless modunda imleç gecikmesi sürüyor | v1.5 kullanıcı gecikmeyi daha düşük hissetti. Kontrollü ölçüm açık |
| INPUT-04 | Araca binmek bazen ikinci basışı gerektiriyor | v1.5 kullanıcı denemesinde tekrarlanmadı. Eski Borderless koşulunda tekrar kontrol açık |
| WIN-01 | Alt-Tab sırasında eski Windows çerçevesi/toolbar parlaması | Açık — kullanıcı bildirdi |
| WIN-02 | Odak kaybında otomatik pause | İstenen davranış / eksik |
| UI-01 | Borderless, Graphics ekran biçiminin üçüncü seçeneği olmalı | OpenGL derlendi / oyun testi bekleniyor |

### SAV-01 — F5/quicksave konumu

**Gözlem:** F5 ile alınan manuel kayıt yüklenince evde başlatıyor. Yeni netleştirme:
F5 kaydı, kaydedildiği konumda devam etmeli; ev konumu yalnızca görev sonrası kayıt akışına ait.
Manuel kayıtta can doldurma **kullanıcı tarafından çalışıyor olarak doğrulandı**; korunmalı.

**Beklenen:** F5 kaydında mevcut konum korunur. Kaydetme oyuncuyu canlı dünyada taşımaz.
Görev sonrası kayıtla aynı konum politikası kullanılmaz. Menü iptali dosya yazmaz.

**Kaynak bulgusu:** `Enhanced.cpp` içinde hem F5 hem görev sonrası akış aynı
`homeSaveRequested` bayrağını açıyor. Ayrı kayıt nedeni/konum politikası gerekecek.

### SAV-02 — görev sonrası kayıt noktasında çatıya çıkma

**Gözlem:** Kayıt doğru adadaki eve gidiyor, fakat doğru kayıt noktası yerine çatıda doğuruyor.

**Beklenen:** Görev başarılı olduktan sonra güvenli anda slot menüsü açılır; kendiliğinden
dosya yazılmaz. Kullanıcı kaydederse yükleme, o adanın **özgün ev kayıt noktasında**,
orada normal kayıt alınmış gibi başlatır. Görev konumu yazılmaz. Üç ada ayrı ele alınır.
Sadece yanlış Z'yi düşürmek yerine doğru X/Y/Z ve ilgili yerleşme davranışı belirlenmeli.

**Kaynak bulgusu:** `EnhancedServices.cpp::FindHomePosition`, yukarıdan
`FindGroundZForCoord(...) + 1` kullanıyor. Çatıyı zemin seçmesi olası neden;
bu, henüz oyun içinde doğrulanmış kök neden değil. Canlı teleport/scene reload çözümü kullanılmamalı.

### SAV-03 — quicksave kısıtlamaları

**İstek ve kanıt:** SS5'teki dört koşul uygulanmalı: oyuncu görevde değil,
araçta değil, hareketsiz ve aranma seviyesi sıfır. Bunlardan biri bozulduğunda
kayıt tetiklenmemeli; anlaşılır geri bildirim olmalı. Mevcut sinematik, ölüm,
replay, açık menü ve güvenli oyun döngüsü kontrolleri korunur.

**Kabul:** Dört engelin her biri ayrı denendiğinde dosya yazılmaz/menü açılmaz;
güvenli yaya durumda F5 çalışır. Görev-sonu kayıt isteği güvenli ana kadar bekler.

### VEH-01 — cam açıkken sürücüye isabet

**Gözlem:** Ön cam kırılıyor, fakat içerideki kişiye hâlâ hasar verilemiyor.
Önceki derlemenin geçmesi bu hatayı kapatmıyor.

**Beklenen:** Gerçek mermi hattı kırık ön cam açıklığından sürücü/yolcuya ulaşırsa
native silah hasarı uygulanır. Her atışta otomatik yolcu hasarı verilmez.
Duvar, kaporta ve başka araç engelleri; bulletproof/görev korumaları korunur.
İlk cam kıran atış ve sonraki atışlar, tamir sonrası ve farklı araç modelleri ayrıştırılır.

**İnceleme:** `EnhancedVehicle.cpp`, `CWeapon::ProcessLineOfSight` ve `BulletInfo`;
oturan ped çarpışma modeli, dünya isabetinin değiştirilmesi ve sonraki native hasar dalları.
Araç/silah bazlı tekrar üretim henüz yok; kök neden kesinleşmedi.

### DIST-01 / DIST-02 — araç ve yaya mesafeleri

**Gözlem:** Kullanıcı, her iki ayar 1× üstüne çıktığında spawn davranışının
bozulduğunu ve 1,5×, 2×, 3× seçeneklerinin etkisinin beklediği gibi görünmediğini
bildirdi. Öncelik spawn akışındaki bozulma. Kesin tetikleyici ve neden araştırılacak.

**Beklenen:** Her seçenek doğma, görünürlük/LOD ve silinme eşiklerini tutarlı etkiler;
hızlı yolculukta önde ani belirme azalır. Native havuz sınırları ve görev varlıkları korunur.
Araç ve yaya ayarları ayrı takip edilir. Bir ayarı büyütmek diğer eşikte anlık silinme yaratmamalı.
1,5× seçeneği kaldırılabilir. Kalan 1×, 2× ve 3× seçeneklerinin gerçek
etkisi ve eski ayar dosyalarındaki değerlerin geçişi kontrol edilmeli.
İstenen ek davranış, ortamda doğal olarak üretilen araç ve yaya adet sınırlarının
ilgili ayarın çarpanıyla ayrı ayrı ölçeklenmesi. Fiziksel havuz kapasitesi ve görev
varlıkları hesaba katılmalı.

**İnceleme:** `CarCtrl.cpp`, `Population.cpp`, `VisibilityPlugins.cpp`; grafik draw
distance ile etkileşim, görünen/görünmeyen varlık eşikleri ve havuz doygunluğu.
Spawn mesafesi ve adet sınırı ayrı incelenmeli. Aynı rota ve koşullarda 1×,
2× ve 3× ile spawn sıklığı, eşikler ve aktif varlık sayısı karşılaştırılmalı.

### GPS-01 — elle işaretlemede üst yol

**Gözlem:** Üst üste gelen yolların katmanı karışıyor.

**Beklenen:** Elle waypoint, kullanıcının üstten haritada gördüğü **en üstteki
uygun yol yüzeyine** bağlanır. Görev hedefinin gerçek Z/konumu biliniyorsa bu
korunur; görev rotasına zorunlu “üst katman” kuralı uygulanmaz. Araç/yaya erişimi
ve bağlı yol ağı geçerliliği ayrıca korunur; en yüksek ilgisiz bina seçilmez.

**Kaynak bulgusu:** `Enhanced.cpp::UpdateRoute` elle hedeflerde aday mesafesini
2D ölçüyor. Aynı X/Y'deki katmanları ayırt etmeyen bu seçim incelenecek.
Uzak/hedef bölgesi yüklenmemişken de tutarlı politika gerekli.

### GPS-02 — boşluk, erken bitiş ve geri dönüş

**Kanıt:** SS3'te rota işarete varmadan bitiyor gibi; SS4'te U dönüşlü/çift hatlı
bir çizim var. Oyuncunun araçta/yaya oluşu ve yol katmanı görüntülerden kesin değil.

**Beklenen:** Oyuncudan seçilen yol ağına, yol boyunca hedef bağlantısına kadar
rota tutarlı olur. Ulaşılamayan/off-road hedef için duvarın içinden sahte düz çizgi
çizilmez; son ulaşılabilir yol noktası ile hedef arasındaki ayrım anlaşılır olur.
Gerçekten gerekli dönüş ile yanlış bağlanma kaynaklı dönüş ayrıştırılır.

**İnceleme:** Başlangıç/hedef düğüm seçimi, yakın hedef aday maliyetleri, yol katmanı,
`ClearRouteConnector` sınırları, rota önbelleği ve oyuncuyu ilk segmente bağlama.
Tek yön/kapalı yol ve görev rengi davranışları korunur.

### MAP-01 — çift ev ikonu

**Kanıt:** SS1 ve SS3 aynı ev yakınında iki mor ikon gösteriyor.

**Beklenen:** Bir ev için tek ikon. Runtime blip, SCM taraması ve servis önbelleği
aynı noktayı birden çok kez çizmemeli. Yakın ama gerçekten farklı noktalar
kör bir mesafe eşiğiyle birleştirilmemeli.

**İnceleme:** `Radar.cpp::DrawBlips` ile `EnhancedServices.cpp::DrawServices`
aynı servisi birlikte çiziyor olabilir; eski/yeni SCM konumları da aday neden.
Görüntü, bunlardan hangisinin tek başına neden olduğunu kanıtlamıyor.

### MAP-02 — keşif ve açılma durumu

**Gözlem:** Henüz açılmamış üçüncü adanın ikonları haritada görünüyor (SS2).
Kullanıcının önerisi: bir kez görülmeyen servis ikonunu göstermemek.

**Beklenen:** Harita ilerlemeyi/keşfi ifşa etmez. İlk kez gerçekten görülen veya
oyunun açtığı servis kaydedilir; ilgili bölgeden uzaklaşınca keşfedilmiş ikon
gerekçesiz kaybolmaz. Görev simgeleri, oyuncu/waypoint ve servisler için aynı
genel gizleme kuralı uygulanmamalı.

**Tasarım kararı açık:** En uygun çözümün keşif + native aktiflik/ilerleme bilgisiyle
seçilmesi istendi. Henüz görülmeyen tüm SCM sabitlerini doğrudan çizmek doğru
değil. Keşif bilgisinin yeni oyun, kayıt yükleme, farklı save ve eski save
uyumluluğunda nasıl saklanacağı kararlaştırılmalı; save biçimi rastgele değiştirilmez.

### MAP-03 — görülen payphone kayboluyor

**Gözlem:** Kullanıcı diğer adada gördüğü payphone'u map menüsünde göremiyor.
SS2'deki 8-ball simgesi payphone kanıtı olarak yorumlanmadı.

**Beklenen:** Görülmüş ve hâlen geçerli payphone haritada doğru simgeyle korunur;
tamamlanmış/pasif görev telefonları native görev kurallarına göre değerlendirilir.
Keşif kuralı telefonu da kapsamalı; her rastgele telefonu görev simgesi yapmamalı.

**Kaynak bulgusu:** Özel servis listesi yalnızca spray/weapon/bomb/save içeriyor.
Payphone'un gerçek script/blip kaynağı ve yaşam döngüsü bulunmalı; ikon türü tahmin edilmemeli.

### MAP-04 / MAP-05 — alt şerit ve Back

**Netleştirme:** İstenen yalnızca kontrol yazılarını kaldırmak değildi.
**Alttaki sarı MAP şeridi ve MAP başlığı tamamen kalkacak.** Harita alanı buna
göre düzenlenecek. Back kalacak; mavi Rockstar logosunun üzerinde aynı sol/X
hizasında olacak. SS3'teki soldan kesilme giderilecek.

**Kabul:** Back tamamen görünür ve tıklanabilir; çizim/tıklama koordinatları aynı
dönüşümü kullanır. 16:9 ve 4:3'te hizalanır. Haritada Windows benzeri oyun imleci
geri gelmez; yatay/dikey crosshair korunur. Şerit kaldırılınca başlık veya metin artığı kalmaz.

### INPUT-01 — iki backend arasında fare hızı

**Gözlem:** OpenGL ve D3D9 aynı hissi/hızı vermiyor.

**Beklenen:** Aynı hassasiyet değeri benzer fiziksel fare hareketinde aynı kamera
açısını üretir. Backend, FPS, çözünürlük ve DPI etkileri ayrıştırılır. Menü imleci
ile oyun kamerası aynı şey sayılmaz; hangisinin etkilendiği incelemede netleştirilir.
Doğrudan sayısal değer girişi ek isteği [RM-04](../ROADMAP.md) altında tutuluyor.

### WIN-01 — Alt-Tab çerçeve/toolbar parlaması

**Gözlem:** Alt-Tab sırasında anlık Windows 7 benzeri toolbar/çerçeve görünüyor.
Bu, gerçek işletim sistemi sürümünün Windows 7 olduğu anlamına gelmez.

**Beklenen:** Borderless geçişinde eski pencere süsü, başlık çubuğu veya toolbar
parlamaz. Native window-style değişimi, focus/minimize/restore ve device reset
akışı incelenir; sadece görüntüye bakılarak sebep kesinleştirilmez.

### WIN-02 — odak kaybında pause

**İstek:** Oyun odağı kaybettiğinde pause'a geçmeli.

**Kabul:** Alt-Tab veya başka pencereye tıklamada oyun simülasyonu durur;
arkada araç sürmeye/hasar almaya devam edilmez. Odak geri gelince önceden açık
manuel menü/pause durumu bozulmaz, yapay tuş basışı oluşmaz. İki backend'in focus
callback/Windows mesajı aynı kullanıcı davranışını sağlamalı. Ses politikası ayrıca değerlendirilir.

### UI-01 — ekran biçiminde üçüncü seçenek

**İstek:** Enhanced içindeki bağımsız Borderless ayarı Graphics'e taşınacak;
mevcut ekran biçimi seçeneği **Fullscreen / Windowed / Borderless** olacak.

**Kabul:** Tek, tutarlı seçim ve ini kaydı. Eski `[Enhanced] Borderless` ve
windowed tercihlerinin geçişi belirlenir; çakışan iki seçenek bırakılmaz.
Native device/window mode gerçekten seçeneği izler. Çalışan oyun sırasında
değiştirme sınırları arayüzde doğru gösterilir. RM-05 backend kararıyla birlikte ele alınır.

## Doğrulananlar ve henüz doğrulanmayanlar

- Kullanıcı: manuel kayıtta can doldurma çalışıyor.
- Kullanıcı: evin adası doğru seçiliyor, fakat doğma yüksekliği yanlış.
- Kullanıcı: D3D9 Ambilight ile fullscreen'de de çalışıyor; nottaki karşılaştırmaya
  göre OpenGL hem fullscreen hem borderless'ta çalışmıyor.
- 19 Kasım 2026 kullanıcı güncellemesi: **D3D9 sürümünde Discord yayın önizlemesi
  kontrol edildi; şu an donma/sorun görünmüyor.** D3D9 için olumlu kullanıcı
  kontrolü olarak kaydedildi. Uzun süreli yayın veya karşı izleyici doğrulaması
  bildirilmedi.
- Aynı gün sonraki kullanıcı kontrolü: **OpenGL sürümünde Discord yakalama/yayın
  çalışmıyor.** OpenGL için artık test sonucu bilinmiyor değil, kullanıcı tarafından
  başarısız bildirildi. Bu kontrolde pencere modu ve hatanın ayrıntısı ayrıca belirtilmedi.
- Son mimari karar: ana geliştirme yolu OpenGL, D3D9 alternatif. Önceki D3D9
  Windows varsayılanı önerisi kullanıcı tercihiyle değiştirildi.
- Yeni netleştirme: Discord normal OpenGL penceresini yakalıyor, fullscreen ve
  borderless'ı yakalamıyor. Ambilight normal pencere sonucu belirtilmedi.
  İnceleme ve aday değişiklik: [CAP-01](OPENGL-CAPTURE.md).
- Intro farkı hata olarak açılmadı: kullanıcı D3D9'da Rockstar/GTA videolarının
  oynadığını, OpenGL'de oynamadığını bildirdi. Kaynakta GLFW video çağrılarının
  kapalı olduğu görüldü. **Skip Intros** özellik isteği RM-06'da takip edilir.
- Önceden listelenen ama bu notta sonucu belirtilmeyen radar ölçekleme, GPS renkleri,
  görev-sonu menünün tek tetiklenmesi, sağlık kaydı ve ASI/CLEO kontrolleri doğrulanmış sayılmaz.

## Önerilen düzeltme sırası

1. SAV-01 → SAV-02 → SAV-03: kayıt nedenini ayır, evin gerçek noktasını düzelt, engelleri koru.
2. VEH-01: camdan sonra gerçek ped isabetinin nerede kaybolduğunu izle.
3. MAP-01 → MAP-02 → MAP-03: tek ikon kaynağı, keşif/ilerleme ve telefon yaşam döngüsü.
4. MAP-04 → MAP-05: şeridi kaldır, Back'i doğru ölçekle hizala.
5. GPS-01 → GPS-02: önce hedef katmanı, ardından bağlantı/rota sürekliliği.
6. DIST-01 → DIST-02: somut belirtiyi netleştirip eşikleri birlikte incele.
7. RM-05 kararı → INPUT-01 → WIN-01 → WIN-02 → UI-01: backend, input ve pencere akışı.

Her maddeye geçildiğinde neden, değişen dosyalar ve kalan belirsizlik buraya eklenir.
Build/test yalnızca kullanıcının ayrıca talep ettiği toplu doğrulama aşamasında yapılır.

## v1.2 OpenGL denemesi — kullanıcı build istedi

Yalnızca CAP-01 başlangıç adayı ve UI-01 üçlü ekran biçimi bu derlemeye
alınıyor. Sürücü profili/global ayar çözüm olarak uygulanmayacak. Diğer
kayıtların düzeltilmesine başlanmadı. Kullanıcı yalnızca OpenGL build istedi;
D3D9 derlemesi yapılmayacak. Runtime sonuçları henüz yok.

OpenGL v1.2 derlemesi ve mevcut birim kontrolleri başarıyla tamamlandı.
D3D9 işi çalıştırılmadı. Yalnızca CAP-01/UI-01 için aday çıktı üretildi;
yakalama ve yeni menünün runtime doğrulaması kullanıcı testini bekliyor.

Sonraki kullanıcı geri bildirimi: v1.2 yakalama sorununu çözmedi. CAP-01 açık;
UI-01 menü davranışı için ayrı bir başarısızlık bildirimi yapılmadı. Sonraki
sunum/kompozisyon denemesi [yakalama notunda](OPENGL-CAPTURE.md) kayıtlıdır.

## v1.3 dar kapsamlı yakalama denemesi

CAP-01 için +1 genişlik probu kullanıcı tarafından denemeye alındı. Yalnızca
Windows OpenGL Borderless etkilenir. Kaynak değişikliği ve tek seferlik
pencere boyutu günlüğü hazır; sürücü ayarı yok. Kayıt/cam/GPS/harita gibi
diğer açık hatalar bu denemede değiştirilmedi. Build ve runtime sonuçları ayrı izlenecek.

v1.3 için tek OpenGL derlemesi ve mevcut birim kontrolleri geçti. CAP-01
yakalama sonucu henüz yok; hata kapanmadı. v1.2 başarısızlık kaydı korunur.

## INPUT-02 — v1.3 Borderless açılış imleci

Kullanıcı +1 borderless ile yakalamanın şimdilik düzeldiğini, ancak bu modda
kapatıp yeniden açınca imlecin sola yapıştığını bildirdi. Fullscreen → Borderless
geçişi geçici olarak düzeltiyor.

Kaynakta `cursorCB` pencere/render oranını tamsayı bölüyor: 1920/1921=0.
İlk açılışta `psPostRWinit`/resize, RwInitialised doğru olmadan; mod değiştirmede
ise doğru olduktan sonra çalışabiliyor. Kesin kaynak kusuru ile runtime boyut
eşitsizliği çıkarımı ayrı tutuldu. `xpos==0` erken dönüşü ve input başlangıcı
da incelenecek. Kod değişmedi; yeni build/test yok.

Gerçek günlük 1921×1080 borderless pencere/framebuffer ve monitorAttached=0
gösteriyor; ini seçimi ScreenFormat=2. +1 tek başına nihai kusursuz çözüm
sayılmayacak. Öncelik INPUT-02, sonra SAV-01/SAV-02.

Tam rapor, kanıtlar, yöntem sınırları ve gerçek fullscreen seçenekleri:
[DISPLAY-REPORT-v1.3.md](DISPLAY-REPORT-v1.3.md).

### SAV-02 için ilk salt okunur script bulgusu

Kurulu main.scm içinde ACTIVATE_SAVE_MENU öncesindeki
IS_PLAYER_IN_AREA_ON_FOOT_3D koşulları okundu. Portland: (891.1875,-309.6875,7.6875)
ile (899.25,-303.25,12.6875); Staunton: (105.6875,-486,15) ile (100.75,-483.375,18);
Shoreside: (-670.5,3.875,18) ile (-660,12.6875,22). Bunlar gerçek native kayıt
bölgeleridir, tek bir kesin spawn koordinatı değildir. Özellikle Shoreside
harita ikonu (-665,-6.5) kayıt bölgesinin Y aralığında değil. Bu, ikon konumunu
spawn noktası yerine kullanmanın da yanlış olabildiğini gösteriyor; yalnızca
çatı Z hesabı değiştirilerek tam çözüm sağlandığı varsayılmamalı. Kod henüz değişmedi.

## v1.4 — imleç düzeltmesi ve bağımsız fullscreen denemesi

INPUT-02 kaynak düzeltmesi ayrı commit'te: 9553703c. Kesirli pencere/render
oranı, son framebuffer ölçülerinin ilk açılışta eşlenmesi, odak dönüşü ve
geçerli x=0 girdisi düzeltildi. Yeni birim regresyonları CI'da çalıştırılacak.

CAP-01 için Windows OpenGL'e opsiyonel WGL_NV_DX_interop2 / DXGI fullscreen
sunumu eklendi. Render OpenGL'dir. İki GPU blit MSAA çözümü ve Y yönünü
ayırır, CPU readback yoktur. Sistem D3D11 dinamik yüklenir; extension/GPU
uyuşmazlığı veya sunum hatasında native GLFW fullscreen'e dönülür ve log yazılır.
Bu donanım bağımlı bir deneydir; capture ve gecikme doğrulanmadı.
FullscreenDXGI=0 ile deneme kapatılır, imleç düzeltmesi korunur. Borderless
+1 yolu korunur. Açık diğer bug'ların durumu değişmedi. Kullanıcı build istedi.

### v1.4 derleme sonucu — 2026-11-19

Kaynak 1432a85c, GitHub Actions run 44444400311: yalnızca OpenGL x64
başarıyla derlendi. Üç mevcut birim programı, yeni imleç oranı regresyonları
dahil geçti. D3D9 işi çalıştırılmadı. INPUT-02 kaynak/derleme doğrulandı;
soğuk açılış, Alt-Tab, CAP-01 fullscreen Discord/Ambilight ve gecikme kullanıcı
denemesini bekliyor. Hiçbir runtime bug bu derleme sonucuyla kapatılmadı.

### v1.4 kullanıcı denemesi — fullscreen siyah ekran

Kullanıcı deneysel fullscreen etkinleştirildiğinde görüntünün tamamen
siyah olduğunu ve ayarı menüden geri alamadığını bildirdi. Günlük DXGI
fullscreen durumuna geçildiğini gösteriyor. Görüntü hatasının nedeni
henüz belirlenmedi. Günlük kanıtı
[burada](feedback/2026-11-19/v1.4-fullscreen-black-screen.txt).

Kurulu oyunun ayarı yedeklendi, FullscreenDXGI=0 ve ScreenFormat=2
uygulandı. Kurtarma sonrası görüntü ve Borderless imleci kullanıcı
doğrulamasını bekliyor. CAP-01 açık.

### INPUT-03 — Borderless imleç gecikmesi

Kullanıcı v1.4'te Borderless modunda imleç gecikmesinin sürdüğünü bildirdi.
INPUT-02, açılışta imlecin sola kilitlenmesiyle ilgili ayrı bir kayıt.
Gecikmeyi karşılaştırmak için Borderless ve Fullscreen aynı FPS ve VSync
ayarlarıyla denenebilir.

### INPUT-04 — Araca binmek için ikinci basış gerekiyor

Kullanıcı Borderless modunda başka bir tuşa basmıyorken araca binme komutunun
ilk basışta bazen takıldığını, ikinci basışta karakterin araca bindiğini
bildirdi. Kullanıcının Fullscreen denemesinde sorun görülmedi.
Ekran modu ile bağlantısı henüz belirlenmedi. Karşılaştırmada aynı araç,
yaklaşma konumu ve tuş kullanılmalı. Beklenen davranış, binme koşulları
sağlandığında tek basışla araca binilmesi.

### Ayrı sohbet devri — CAP-01, INPUT-03 ve INPUT-04

- **CAP-01:** OpenGL normal pencere Discord'da çalışıyordu. Borderless +1 piksel
  kullanıcıda şimdilik olumlu. Gerçek Fullscreen'de Discord/Ambilight yakalaması
  açık sorun. v1.4 DXGI denemesi tamamen siyah ekran verdi. Günlük DXGI fullscreen
  geçişini kaydetti. Kurulu oyun `FullscreenDXGI=0` ve `ScreenFormat=2` ile
  kurtarıldı. Önce [deneme günlüğünü](feedback/2026-11-19/v1.4-fullscreen-black-screen.txt)
  ve [yakalama notlarını](OPENGL-CAPTURE.md) oku. Oyun içinde uygulanabilir
  çözüm ve menü erişilemezse geri dönüş yolu gerekli.
- **INPUT-03:** Borderless imleç gecikmesi v1.4'te sürüyor. Açılışta sola
  yapışmayı hedefleyen INPUT-02 kaynak düzeltmesi ayrı commit'te (`9553703c`).
  Gecikmeyi aynı çözünürlük, FPS ve VSync ile karşılaştır.
- **INPUT-04:** İlk araç giriş komutu bazen takılıyor, ikinci basışta biniyor.
  Borderless'ta görüldü, kullanıcının Fullscreen denemesinde görülmedi. İlişki
  belirsiz. Tuş olayını ve araç giriş durumunu aynı koşullarda ayrı izle.
- Bu sohbetin kapsamı bu üç madde. DIST-01 ve DIST-02 ayrı takip edilecek.


### Bu sohbette yapılan kaynak incelemesi

**CAP-01.** v1.4 günlüğü DXGI exclusive durumuna geçişi ve 1920×1080
framebuffer'ı kaydediyor. Önceden mevcut GL hatası olarak kaydedilen tek
`0x501` satırı, siyah karenin oluştuğu aşamayı belirlemiyor. Yeni adayda
WGL, swap-chain backbuffer yerine aynı boyut ve biçimde ayrı bir D3D11
render texture'a yazıyor. WGL kilidi açıldıktan sonra kare GPU üzerinde
`CopyResource` ile backbuffer'a aktarılıyor. Günlükte
`transfer=texture-copy` yeni yolu işaretliyor. API hatalarında mevcut
native OpenGL dönüşü geçerli. Siyah ekran, Discord ve Ambilight sonuçları
ayrı ayrı açık. [Microsoft D3D11 CopyResource](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-copyresource)

Yeni varsayılan `FullscreenDXGI=0`. Mevcut INI'deki açık tercih okunuyor.
Yeni Windows OpenGL EXE'yi `-safe-display` ile başlatmak, pencere
oluşturulmadan önce DXGI'yi kapatır ve `ScreenFormat=2` seçer. Oyun
ayarları kaydedilirse güvenli değerler INI'ye de yazılabilir. v1.4 için
menüsüz kurtarma, oyun kapalıyken `re3.ini` dosyasındaki `[VideoMode]`
bölümünde `FullscreenDXGI=0` ve `ScreenFormat=2` ayarlarıdır.

**INPUT-03.** Menü imleci GLFW callback koordinatıyla oyun karesine
çiziliyor. Oyun içi fare hareketi render karesinde örnekleniyor ve mantık
karesine aktarılıyor. Windows OpenGL'de `-trace-display-timing`, her
120 karede gerçek FPS'yi, framebuffer boyutunu, FPS limitini, VSync'i,
menü durumunu ve sunum çağrısının ortalama süresini `re3-display.log`
dosyasına yazıyor. Uçtan uca gecikme için fiziksel hareket ve ekran
tepkisi aynı yöntemle görüntülenmeli.

İlk karşılaştırmada aynı monitör, yenileme hızı, sahne, framebuffer
çözünürlüğü, FPS ve VSync kullanılmalı. Eşit framebuffer için
`BorderlessCaptureProbe=0`, `FullscreenDXGI=0` ve iki modda monitörün
çözünürlüğü kullanılmalı. Borderless +1 ayrı turda ölçülmeli. Menü imleci
ve oyun içi kamera ayrı gözlenmeli. INPUT-02 koordinat oranı ve başlangıç
eşitlemesi korunuyor. Gecikme ölçümü bekleniyor.

**INPUT-04.** GLFW basışı ve bırakışı geçici klavye durumuna iletiyor.
İkisi aynı mantık karesi örnekleme aralığına düşerse kısa basış pad
kenarına dönüşmeyebilir. Yaya araç girişi `ExitVehicleJustDown()` ile
başlıyor. Tuş kenarı alınsa bile araç araması, giriş hedefi, kapı ve ped
durumu sonucu belirleyebilir. Bildirilen olayın hangi aşamada gerçekleştiği
ve ekran moduyla ilişkisi açık.

GLFW EXE'de `-trace-vehicle-entry`, bağlı klavye tuşunun basış ve bırakışını,
pad kenarını, araç aramasını ve takip eden ped/hedef/kapı değişimlerini
`re3-vehicle-entry.log` dosyasına kaydediyor. Aynı araç, kapı, yaklaşma
konumu ve tuşla başarılı ve aksayan deneme karşılaştırılmalı. Giriş
kuralları bu aşamada aynen çalışıyor.

Bu değişikliklerin derleme ve oyun içi doğrulaması bekliyor.

### v1.5 derleme sonucu

Kaynak 56b63808, GitHub Actions run 44447245429. OpenGL x64
derlemesi ve uc mevcut birim programi basarili. Windows oyun ici goruntu,
Discord, Ambilight, imlec gecikmesi ve arac girisi dogrulamasi bekliyor.

### v1.5 kullanıcı geri bildirimi ve geçiş düzeltmesi

- **CAP-01:** Kullanıcı DXGI Fullscreen'de hem Discord oyun penceresi
  paylaşımının hem de tüm ekran paylaşımının çalıştığını bildirdi. Açılışta
  ve Alt-Tab sırasında başlık çubuğu, sol üstte beyaz alan ve geri kalan
  alanda siyah görüntü beliriyor. Geçiş dışında sorun bildirilmedi.
  Ambilight sonucu bekliyor. [Ekran görüntüsü](feedback/2026-11-19/v1.5-fullscreen-startup-alt-tab.png)
- **INPUT-03:** Kullanıcı gecikmeyi daha düşük hissetti. Aynı framebuffer,
  FPS ve VSync ile ölçüm sonucu henüz yok.
- **INPUT-04:** Kullanıcı v1.5 denemesinde araç giriş hatasının düzeldiğini
  bildirdi. v1.5 giriş kurallarını değiştirmediğinden kök neden ve önceki
  Borderless koşulundaki tekrar durumu açık.

DXGI, pencereyi oluşturduktan sonra ilk sunum çağrısında exclusive moda
geçiyordu. Alt-Tab çıkış işlemi de sunum çağrısına bağlıydı. Ekran
görüntüsündeki ara pencereyi hedefleyen kaynak adayı, DXGI modunda WGL
penceresini gizli ve çerçevesiz oluşturuyor. Pencereyi göstermeden önce
bir siyah kare hazırlıyor. Odağın kaybı veya simge durumuna geçiş,
olay kuyruğu işlendikten sonra DXGI fullscreen'den çıkışı tetikliyor.
Açılışta veya geri odaklanırken framebuffer boyutu değişirse eski
boyutta çizilen kare DXGI'ye aktarılmıyor. Görsel düzeltme henüz derlenmedi
ve oyun içinde doğrulanmadı.

Geçiş doğrulanınca Windows OpenGL'de DXGI sunumunu ana Fullscreen
seçeneğine bağlama kararı değerlendirilecek. Bu denemede ayrı deney
seçeneği, native OpenGL fallback'i ve `-safe-display` kurtarması duruyor.


### v1.6 derleme sonucu

Kaynak a7ab4d76, GitHub Actions run 44448899982. OpenGL x64
derlemesi ve mevcut birim kontrolleri geçti. Açılış ve Alt-Tab görüntüsü,
Discord yakalama regresyonu ve Ambilight oyun içinde doğrulanmayı bekliyor.

### v1.6 kullanıcı sonucu — monitör kararması

Kullanıcı DXGI Fullscreen açılışında ve Alt-Tab sırasında tüm monitörün
karardığını bildirdi. Kararmanın süresi ve bu sürümdeki yakalama sonucu
belirtilmedi. v1.5 ekran görüntüsünde pencere içi beyaz ve siyah alanlar vardı.

Sonraki kaynak adayı v1.6'nın geçişte OpenGL üzerinden sunduğu siyah
kareleri kaldırıyor. Oyun çözünürlüğü masaüstü çözünürlüğüyle eşleşiyorsa
DXGI mod değiştirme bayrağı kapalı ve `ResizeTarget` çağrılmıyor. Farklı
çözünürlükte `ResizeTarget` exclusive geçişten önce çağrılıyor ve yenileme
hızı DXGI'ye bırakılıyor. Günlükte `modeSwitch=0`, uygulamanın çözünürlük
değiştirmeyi istemediği yolu gösteriyor. Kararmanın nedeni henüz
doğrulanmadı. [Microsoft DXGI geçiş belgeleri](https://learn.microsoft.com/en-us/windows/win32/direct3darticles/dxgi-best-practices)

İlk deneme monitörün masaüstü çözünürlüğünde soğuk açılış ve Alt-Tab
olmalı. Ardından iki Discord paylaşım türü tekrar kontrol edilmeli.
`-safe-display` ile Borderless kurtarması kullanılabilir. Ana Fullscreen
seçeneğine geçiş, görüntü ve yakalama doğrulamasını bekliyor.

### v1.7 derleme sonucu

Kaynak 0ad073c2, GitHub Actions run 44449950521. OpenGL x64
derlemesi ve mevcut birim kontrolleri geçti. Monitör kararması ve yakalama
sonuçları oyun içinde doğrulanmayı bekliyor.

### v1.7 kullanıcı sonucu ve v1.8 gecikme adayı

İlk açılışta kararma sürüyor. Kullanıcı bunun önceki beyaz bölmeli
pencereden farklı göründüğünü ve Alt-Tab geçişinde gecikme hissettiğini
bildirdi. v1.7 kullanıcı için kabul edilebilir bir geri dönüş sürümü.

Aynı çözünürlükte v1.8, DXGI Alt-Tab çıkışında GPU tamponlarını ve WGL
interop kaydını tutuyor. Dönüşte framebuffer boyutu aynıysa bunları
yeniden kullanıyor. Boyut değişirse tamponlar yeniden hazırlanıyor.
Gerçek fullscreen geçişi sürüyor. Günlükte dönüşte `reusedBuffers=1`
bekleniyor. `enterMs` ve `exitMs` uygulama içi DXGI geçiş süresidir.
Monitörün görüntüye dönüş süresi ve ilk açılış kararması ayrı gözlenmeli.

### v1.8 derleme sonucu

Kaynak 8454bb1d, GitHub Actions run 44450629395. OpenGL x64
derlemesi ve mevcut birim kontrolleri geçti. Alt-Tab gecikmesi, açılış
kararması ve Discord yakalaması oyun içinde doğrulanmayı bekliyor.

### v1.8 kullanıcı sonucu — Alt-Tab beklemesi sürüyor

Kullanıcı v1.8 Fullscreen geçişinde yaklaşık 1–1,5 saniye beklediğini
bildirdi. Tamponları yeniden kullanmak, hissedilen beklemeyi gidermedi.
Bu denemeye ait geçiş günlüğü henüz mevcut değil.

Yeni kaynak adayı `-dxgi-composed-fullscreen` ile denenebilir.
Fullscreen Capture açık ve Screen Format Fullscreen iken OpenGL oyunu
çizmeye devam ediyor. DXGI aynı doku kopyasını monitörü kaplayan
çerçevesiz pencerede sunuyor. Bu yolda exclusive geçiş yapılmıyor.
DXGI kurulamazsa OpenGL çerçevesiz pencereyi kullanıyor. Başarılı kurulum
günlüğünde `composed=1` görülür. Parametresiz açılış mevcut exclusive
davranışı seçer. `-safe-display` de kullanılabilir.

Soğuk açılış ve Alt-Tab süreleri ile Discord pencere ve tüm ekran
paylaşımı, ardından Ambilight oyun içinde doğrulanmalı. Derleme bekliyor.
