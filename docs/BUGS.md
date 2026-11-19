# v1.1 kullanıcı geri bildirimi — hata ve davranış kayıtları

Tarih: 19 Kasım 2026. Kaynak: [özgün notlar](feedback/2026-11-19/original-notes.txt)
ve aşağıdaki beş ekran görüntüsü. Dosya v1.1 ile başlayan geri bildirimi,
sonraki düzeltmeleri ve doğrulama durumlarını kaydeder.

## Çalışma ve doğrulama kuralı

- Önce kayıtlar tutulur, ardından maddeler tek tek ele alınır.
- Kullanıcı açıkça **“build al”** demeden derleme, test paketi, workflow dispatch
  veya otomatik build başlatan push yapılmaz. Her değişiklikte yeniden build alınmaz.
- Durumlar: **açık**, **inceleme gerekiyor**, **kaynakta tamamlandı / derlenmedi**,
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
| SAV-01 | F5 kaydı kendi konumunu korumalı | Kaynak değişikliği hazır — oyun doğrulaması bekliyor |
| SAV-02 | Görev sonrası evde çatıda doğma | Kaynak değişikliği hazır — üç ada oyun doğrulaması bekliyor |
| SAV-03 | Quicksave engelleme koşulları | Kaynakta koşullar mevcut, uyarı güncellendi — oyun doğrulaması bekliyor |
| SAV-04 | Görev sonrası 8. slota sessiz autosave ve kayıt sonrası iyileşme | Kaynak değişikliği hazır — oyun doğrulaması bekliyor |
| VEH-01 | Kırık ön camdan sürücü hasar almıyor | Açık — kullanıcı bildirdi |
| DIST-01 | Vehicle Distance 1× üzerindeyken araç spawn akışı bozuluyor | Kaynak değişikliği hazır — oyun doğrulaması bekliyor |
| DIST-02 | Pedestrian Distance 1× üzerindeyken yaya spawn akışı bozuluyor | Kaynak değişikliği hazır — oyun doğrulaması bekliyor |
| GPS-01 | Elle waypoint'te üst/alt yol katmanı karışıyor | Kapalı — kullanıcı doğruladı |
| GPS-02 | Rota görünürlüğü, boşluklar ve anlamsız geri dönüşler | Kapalı — kullanıcı doğruladı |
| GPS-03 | Haritada su üstünde görünen yer altı yolu | Kapalı — kullanıcı geçişin yer altı yolu olduğunu doğruladı |
| MAP-01 | Çift ev ikonu | Kaynakta tamamlandı / v1.10'da derlendi / oyun testi bekliyor |
| MAP-02 | Hizmet ikonlarının keşfi, mesafesi ve hıza göre opaklığı | Kaynakta tamamlandı / son opaklık düzeltmesi derlenmedi |
| MAP-03 | Telefon ve diğer görev ikonları keşif filtresine takılıyor | Kaynakta tamamlandı / v1.12'de derlendi / son görünürlük kuralı MAP-07'de |
| MAP-04 | Sarı MAP şeridinin tamamı kaldırılmalı | Kaynakta tamamlandı / v1.10'da derlendi / oyun testi bekliyor |
| MAP-05 | Back dikey hiza ve Quit Game'e taşan tıklama | Kaynakta tamamlandı / v1.11'de derlendi / oyun testi bekliyor |
| MAP-06 | M kısayolu, ilk yakınlaştırma ve her açılışta oyuncuya odaklanma | Kaynakta tamamlandı / son açılış düzeltmesi derlenmedi |
| MAP-07 | Ev, telefon ve görev simgelerinde büyük harita ve minimap ayrımı | Kaynakta tamamlandı / derlenmedi |
| CAP-01 | OpenGL fullscreen/borderless yakalama | Kapalı — kullanıcı doğruladı |
| INPUT-02 | Borderless kayıtlıyken yeniden açılışta imleç sola kilitleniyor | Kapalı — kullanıcı doğruladı |
| INPUT-03 | Borderless modunda imleç gecikmesi sürüyor | Kapalı — kullanıcı doğruladı |
| INPUT-04 | Araca binmek bazen ikinci basışı gerektiriyor | Kapalı — kullanıcı doğruladı |
| WIN-01 | Alt-Tab sırasında eski Windows çerçevesi/toolbar parlaması | Kapalı — kullanıcı artık görmediğini bildirdi |
| WIN-02 | Odak kaybında pause, dönüşte Pause menüsünde kalma | Kaynak değişikliği hazır — oyun doğrulaması bekliyor |
| UI-03 | Restore Defaults onayı ve hizmet blip saydamlığı | Kaynak değişikliği hazır — oyun doğrulaması bekliyor |
| UI-01 | Borderless, Graphics ekran biçiminin üçüncü seçeneği olmalı | Kapalı — kullanıcı doğruladı |

### SAV-01 — F5/quicksave konumu

**Gözlem:** F5 ile alınan manuel kayıt yüklenince evde başlatıyor. Yeni netleştirme:
F5 kaydı, kaydedildiği konumda devam etmeli; ev konumu yalnızca görev sonrası kayıt akışına ait.
Manuel kayıtta can doldurma **kullanıcı tarafından çalışıyor olarak doğrulandı**; korunmalı.

**Beklenen:** F5 kaydında mevcut konum korunur. Kaydetme oyuncuyu canlı dünyada taşımaz.
Görev sonrası kayıtla aynı konum politikası kullanılmaz. Menü iptali dosya yazmaz.

**Kaynak değişikliği:** F5 mevcut oyuncu konumunu native kayıt yolundan yazar.
Yalnızca görev sonrası slot menüsü ev konumunu kayda geçirir. Menü kapanınca
ev isteği temizlenir. Can doldurma mevcut manuel kayıt yolunda kalır.

### SAV-02 — görev sonrası kayıt noktasında çatıya çıkma

Sonraki kullanıcı isteğiyle görev sonrası slot seçimi SAV-04'teki otomatik
8. slot akışına taşındı. Bu bölüm ilk bildirimin bağlamını korur.

**Gözlem:** Kayıt doğru adadaki eve gidiyor, fakat doğru kayıt noktası yerine çatıda doğuruyor.

**Beklenen:** Görev başarılı olduktan sonra güvenli anda slot menüsü açılır; kendiliğinden
dosya yazılmaz. Kullanıcı kaydederse yükleme, o adanın **özgün ev kayıt noktasında**,
orada normal kayıt alınmış gibi başlatır. Görev konumu yazılmaz. Üç ada ayrı ele alınır.
Sadece yanlış Z'yi düşürmek yerine doğru X/Y/Z ve ilgili yerleşme davranışı belirlenmeli.

**Kaynak değişikliği:** Üç adanın özgün SCM yaya kayıt kutuları kullanılıyor.
Konum kutunun ortasından hesaplanıyor. Orta yükseklikten aşağı bulunan zemin
ayakta durma yüksekliğini kutu içinde tutuyorsa Z buna ayarlanıyor. Sonuç
alınamazsa kutunun merkezi kullanılıyor. Oyuncu ve yükleme kamerası konumları
kayıt verisine yazılıyor. Üç adadaki kesin iniş noktası oyun içinde denenmeli.

### SAV-03 — quicksave kısıtlamaları

**İstek ve kanıt:** SS5'teki dört koşul uygulanmalı: oyuncu görevde değil,
araçta değil, hareketsiz ve aranma seviyesi sıfır. Bunlardan biri bozulduğunda
kayıt tetiklenmemeli; anlaşılır geri bildirim olmalı. Mevcut sinematik, ölüm,
replay, açık menü ve güvenli oyun döngüsü kontrolleri korunur.

**Kabul:** Dört engelin her biri ayrı denendiğinde dosya yazılmaz/menü açılmaz;
güvenli yaya durumda F5 çalışır. Görev-sonu kayıt isteği güvenli ana kadar bekler.

**Kaynak durumu:** Görev, araç, hareket ve aranma kontrolleri mevcut güvenlik
kapısında korunuyor. F5 uyarısı dört koşulu da söylüyor. F5 ve görev sonrası
istek aynı karede uygunsa F5 önce açılıyor, görev sonrası istek bekliyor.

### SAV-04 — görev sonrası otomatik kayıt

Mission Autosave açıkken görev başarı bildirimi bir kayıt isteği oluşturuyor.
Güvenli yaya durumuna ve açık menü olmamasına kadar bekliyor. Son slot (8)
kullanılıyor. Kayıt tamamlanınca sol üstte `Autosaved.` gösteriliyor.
İşlem hata verirse eski slot korunuyor ve başarısızlık bildiriliyor.
`Heal After Saving` hem manuel hem otomatik kayda uygulanıyor.
Otomatik kayıt manuel kaydın altı saat ilerletme işlemini çağırmıyor.

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
1,5× seçeneği kaldırıldı. Kalan 1×, 2× ve 3× seçeneklerinin gerçek
etkisi ve eski ayar dosyalarındaki değerlerin geçişi kontrol edilmeli.
İstenen ek davranış, ortamda doğal olarak üretilen araç ve yaya adet sınırlarının
ilgili ayarın çarpanıyla ayrı ayrı ölçeklenmesi. Fiziksel havuz kapasitesi ve görev
varlıkları hesaba katılmalı.

**İnceleme:** `CarCtrl.cpp`, `Population.cpp`, `VisibilityPlugins.cpp`; grafik draw
distance ile etkileşim, görünen/görünmeyen varlık eşikleri ve havuz doygunluğu.
Spawn mesafesi ve adet sınırı ayrı incelenmeli. Aynı rota ve koşullarda 1×,
2× ve 3× ile spawn sıklığı, eşikler ve aktif varlık sayısı karşılaştırılmalı.

**Kaynak değişikliği:** Araç doğma yol uzunluğu eşiği araç çarpanını kullanıyor.
Uzatılmış ekran dışı silme aralığı etkinse yüksek ayarlardaki uzak araç adayları
bu aralıkta kabul edilebiliyor. Yaya araması her aday için görünürlük aralığını
yeniden seçiyor ve zemini doğrulanamayan adaydan sonra aramaya devam ediyor.
Her türün 1× tam sayı adet sınırı hesaplanıyor ve kendi ayarına göre 2 veya 3
ile çarpılıyor. Fiziksel havuz sınırı ayrıca uygulanıyor.
Havuzda ambient üretimden ayrılan 16 araç ve 24 ped slotu var. Yaya grupları
kalan bütçeye sığıyor. Eski INI dosyasındaki 2× ve 3× seçimleri korunuyor.
Eski 1,5× seçimi 1× olarak açılıyor.

**v1.13 kullanıcı sonucu:** Uzak varlıklar daha kalıcı, ancak yüksek Distance
ayarında yakındaki trafik seyreliyor ve ilk açılışta sokaklar boş kalabiliyor.
Yeni kaynakta 1× üretim bandı ve buna ait yakın trafik bütçesi tutuluyor.
Ek kapasite yüksek Distance ayarlarında uzak banda gidiyor. Başlangıçta 45
mantık karesi boyunca kare başına en fazla dört yakın üretim denemesi yapılıyor.
Model, görüş, çarpışma, Density ve havuz kontrolleri geçerli.
`-trace-traffic`, `re3-traffic.log` dosyasına iki saniyelik oyun zamanı
aralıklarında yakın/uzak başarı ve deneme sayılarını, sahnedeki yakın/toplam
sayıları, Distance, Density ve konumu yazıyor. Aynı kayıt, ayar ve rotada
soğuk açılış ile sürüş ayrı karşılaştırılmalı. Oyun ölçümü bekliyor.

### GPS-01 — elle işaretlemede üst yol

**Derleme:** v1.10 Windows OpenGL x64 derlemesi ve mevcut geometri, rota,
SCM birim testleri GitHub Actions'da geçti. Oyun testi bekliyor.
[Derleme 44454220355](https://github.com/cnrgks/hezkore-re3-pdb-build/actions/runs/44454220355)

**Gözlem:** Üst üste gelen yolların katmanı karışıyor.

**Beklenen:** Elle waypoint, kullanıcının üstten haritada gördüğü **en üstteki
uygun yol yüzeyine** bağlanır. Görev hedefinin gerçek Z/konumu biliniyorsa bu
korunur; görev rotasına zorunlu “üst katman” kuralı uygulanmaz. Araç/yaya erişimi
ve bağlı yol ağı geçerliliği ayrıca korunur; en yüksek ilgisiz bina seçilmez.

**Kaynak bulgusu:** `Enhanced.cpp::UpdateRoute` elle hedeflerde aday mesafesini
2D ölçüyor. Aynı X/Y'deki katmanları ayırt etmeyen bu seçim incelenecek.
Uzak/hedef bölgesi yüklenmemişken de tutarlı politika gerekli.

**Kaynak değişikliği:** Elle tıklama yol bağlantıları üzerine izdüşürülüyor.
En yakın yol 8 birim içindeyse benzer X/Y'deki üst yol tercih ediliyor. Üst
yol seçildiğinde arama aynı yükseklikteki yakın bağlantıları deniyor.
Görev konumunun geçerli Z değeri korunuyor. Katman seçimi aracın/yayanın
yol ağına dayanıyor ve uzaktaki hedef için de kullanılabiliyor.

### GPS-03 — haritada su üstünde görünen yer altı yolu

Kullanıcı, ekrandaki geçişin bir yer altı yolu olduğunu doğruladı.
Bu bildirim için eklenen ada ilerleme ve yakın bariyer filtreleri geri alındı.
[Ekran görüntüsü](feedback/2026-11-19/v1.13-gps-crossing.png)

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

**Kaynak değişikliği:** Başlangıç ve bitiş yol segmentinin üzerinde hesaplanıyor.
Oyuncu ve hedef aynı yönlü bağlantı üzerindeyse rota aradaki kısmı kullanabiliyor.
Yol dışı son parça yalnızca yakın ve çarpışma verisiyle açık görülen bir
bağlantıda çiziliyor. SS3 ve SS4 konumlarında oyun içi kontrol bekliyor.

**v1.10 geri bildirimi:** Waypoint konduğunda rota görünmedi. Kurulu INI'de
`GPS=0`; bu ayar v1.10'da elle konan waypoint'i de engelliyordu. v1.11'de
elle waypoint konması rotayı doğrudan etkinleştiriyor. Ayar otomatik görev
rotasına ait. Hedefin en yakın bağlantısı erişilemiyorsa arama yakındaki
başka bağlantıları da deniyor.

**v1.11 derleme:** Windows OpenGL x64 ve mevcut geometri, rota, SCM
birim testleri geçti. Oyun ve kayıt yükleme testi bekliyor.
[Derleme 44456718827](https://github.com/cnrgks/hezkore-re3-pdb-build/actions/runs/44456718827)

### MAP-01 — çift ev ikonu

**Kanıt:** SS1 ve SS3 aynı ev yakınında iki mor ikon gösteriyor.

**Beklenen:** Bir ev için tek ikon. Runtime blip, SCM taraması ve servis önbelleği
aynı noktayı birden çok kez çizmemeli. Yakın ama gerçekten farklı noktalar
kör bir mesafe eşiğiyle birleştirilmemeli.

**İnceleme:** `Radar.cpp::DrawBlips` ile `EnhancedServices.cpp::DrawServices`
aynı servisi birlikte çiziyor olabilir; eski/yeni SCM konumları da aday neden.
Görüntü, bunlardan hangisinin tek başına neden olduğunu kanıtlamıyor.

**Kaynak değişikliği:** Ev ikonu etkin oyun blip'inden geliyor. SCM taraması
ev konumu bilgisini kayıt özelliği için saklıyor. Çizim döngüsü temizlenmiş
blip yuvalarını atlıyor.

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

**Kısmi kaynak değişikliği:** Etkin koordinat servis blip'lerine ada açılma
kontrolü eklendi. Garajlar ve silah satış noktaları oyuncu yakınından geçince
oturum önbelleğine giriyor. Bu önbellek kayıt yüklemede ve yeni oyunda
sıfırlanıyor. Kayıtlar arası keşif kalıcılığı henüz uygulanmadı.

**v1.11 kaynak değişikliği:** Keşif, PC kayıt dolgusundaki sürümlü bir
alana yazılıyor ve o kayıt yüklenirken geri alınıyor. Eski kayıtlar boş
keşif verisiyle açılıyor. Ammu-Nation, 8-Ball/bomba garajı ve Pay 'n' Spray
minimap alanına girdiğinde görünüyor. Görev hedefleri keşif ve mesafe
filtresini atlıyor. Kullanıcı kaydında yazma ve yeniden yükleme denemesi
bekliyor.

**v1.12 kapsam düzeltmesi:** Keşif, minimap sınırı ve hıza göre saydamlık
yalnızca Ammu-Nation, 8-Ball bomba garajı ve Pay ’n’ Spray hizmetlerine
uygulanır. Hizmet minimap alanına girdiğinde keşfedilir. Önceki kayıtlardan
yalnızca bu hizmetlerin keşif girişleri yüklenir. Görev için hizmet
simgesiyle işaretlenmiş hedefler yerleşik davranışı kullanır.

**Sonraki düzeltme:** Hizmet simgelerinin opaklığı araç hızıyla artar.
Düşük hızda alfa 160, normal radarın üst hız sınırında alfa 255. Büyük
haritadaki hizmet simgeleri tam opak çizilir.

### MAP-03 — görülen payphone kayboluyor

**Gözlem:** Kullanıcı daha önce gördüğü mavi telefon simgesini harita
menüsünde artık göremiyor.
SS2'deki 8-ball simgesi payphone kanıtı olarak yorumlanmadı.

**Beklenen:** Görülmüş ve hâlen geçerli payphone haritada doğru simgeyle korunur;
tamamlanmış/pasif görev telefonları native görev kurallarına göre değerlendirilir.
Keşif kuralı telefonu da kapsamalı; her rastgele telefonu görev simgesi yapmamalı.

**Bulgular:** Kurulu HUD dokusundaki mavi ahize `radar_el` (sprite 8).
Kurulu SCM'de bu sprite bir görev telefonu için atanıyor. Bir kayıtta
telefon hâlâ çalma durumundayken bu koordinattaki radar kaydı temizlenmiş.
[Salt okunur inceleme](feedback/2026-11-19/map-03-blue-phone.txt)

**Kaynak değişikliği:** Harita, etkin telefonun daha önce atanmış simgesini
radar kaydından hatırlıyor. Bir radar kaydı bulunamayan telefon oyuncu
yakınından geçtiğinde kendi SCM simgesiyle keşfedilebiliyor. Yedek çizim ada
açılma, aktif görev ve telefonun kullanılabilirlik durumunu kontrol ediyor.
Yerleşik simge hâlâ etkinse çizim ona bırakılıyor.

**v1.10 kullanıcı denemesi:** Mavi telefon keşfedildi ve menü haritasında
göründü. Minimapte görünmedi. v1.11'de yedek çizim minimap içinde de çalışıyor.

**v1.12 kapsam düzeltmesi:** Kullanıcı telefon görevlerinin Marty Chonks
görevleri olduğunu ve yerleşik görev simgesi gibi uzaktan görünmesi
gerektiğini bildirdi. Telefon ve diğer görev simgeleri keşif sisteminden
çıkarıldı. Oyunun kendi görev simgesi ve yaşam döngüsü kullanılıyor.
8-Ball görev kontağı da bomba garajı hizmetinden ayrı tutuluyor.

### MAP-04 / MAP-05 — alt şerit ve Back

**Netleştirme:** İstenen yalnızca kontrol yazılarını kaldırmak değildi.
**Alttaki sarı MAP şeridi ve MAP başlığı tamamen kalkacak.** Harita alanı buna
göre düzenlenecek. Back kalacak; mavi Rockstar logosunun üzerinde aynı sol/X
hizasında olacak. SS3'teki soldan kesilme giderilecek.

**Kabul:** Back tamamen görünür ve tıklanabilir; çizim/tıklama koordinatları aynı
dönüşümü kullanır. 16:9 ve 4:3'te hizalanır. Haritada Windows benzeri oyun imleci
geri gelmez; yatay/dikey crosshair korunur. Şerit kaldırılınca başlık veya metin artığı kalmaz.

**Kaynak değişikliği:** Şerit ve harita başlığı kaldırıldı. Back sola hizalı
çiziliyor ve tıklama alanı aynı ekran koordinatlarından hesaplanıyor. 16:9 ve
4:3 oyun içi kontrolü bekliyor.

**v1.10 kullanıcı denemesi:** Back dikeyde hâlâ hizasızdı ve tıklama
Quit Game'i de tetikliyordu. v1.11'de etiket kendi alanında dikey
ortalanıyor. Harita girişi Back tıklamasını fare bırakıldığında tüketip
duraklatma menüsüne dönüyor.
[Kullanıcı görüntüsü](feedback/2026-11-19/v1.10-map-back.png)

### MAP-06 — oyun içinden harita kısayolu

Menu Map açıkken oyun sırasında M haritayı açar. Bu şekilde açılan
harita M, Esc veya Back ile doğrudan oyuna döner. Duraklatma menüsünden
açılan harita Back ve Esc ile duraklatma menüsüne döner.

**Derleme:** v1.12 Windows OpenGL x64 ve mevcut CI birim testleri geçti.
Oyun içinde tuş, görev ikonları ve hizmet saydamlığı denemesi bekliyor.
[Derleme 44458523561](https://github.com/cnrgks/hezkore-re3-pdb-build/actions/runs/44458523561)

**Sonraki düzeltme:** M ve duraklatma menüsü aynı açılış işlevini
kullanıyor. Her açılışta yeniden oyuncuya odaklanılıyor ve başlangıç ölçeği
ekran yüksekliğinin 2 katından 1,65 katına indiriliyor.

### MAP-07 — büyük harita ve minimap simgeleri

Minimapte yalnızca oyuncunun bulunduğu adanın açılmış evi görünür.
Büyük haritada açılmış evler birlikte görünür ve etkin görev kontakları
görev sırasında da gösterilir. Yerleşik telefon simgelerinde radarın
bulunulan adaya göre bir çizim filtresi yok. Telefon, görevi hâlâ
sunuyorsa ve etkin radar kaydı yoksa özgün SCM simgesi kullanılabilir.
Minimap oyunun görev sırasında kontakları gizleme kuralını korur.

### WIN-01 — Alt-Tab çerçeve/toolbar parlaması

**Son durum:** Kullanıcı hatanın artık görülmediğini bildirdi. Kayıt kapatıldı.

**Gözlem:** Alt-Tab sırasında anlık Windows 7 benzeri toolbar/çerçeve görünüyor.
Bu, gerçek işletim sistemi sürümünün Windows 7 olduğu anlamına gelmez.

**Beklenen:** Borderless geçişinde eski pencere süsü, başlık çubuğu veya toolbar
parlamaz. Native window-style değişimi, focus/minimize/restore ve device reset
akışı incelenir; sadece görüntüye bakılarak sebep kesinleştirilmez.

### WIN-02 — odak kaybında pause

**Son kullanıcı isteği:** Odak dönünce oyun Pause menüsünde kalmalı. Kaynakta
odak kaybı simülasyonu durduruyor. Oyun sırasında odak geri geldiğinde
Pause menüsü açılıyor ve manuel devam bekleniyor. Açık menü sayfası korunuyor.

**İstek:** Oyun odağı kaybettiğinde pause'a geçmeli.

**Kabul:** Alt-Tab veya başka pencereye tıklamada oyun simülasyonu durur;
arkada araç sürmeye/hasar almaya devam edilmez. Odak geri gelince önceden açık
manuel menü/pause durumu bozulmaz, yapay tuş basışı oluşmaz. İki backend'in focus
callback/Windows odak durumu aynı kullanıcı davranışını sağlamalı.

**Kaynak değişikliği:** Odak kaybı veya küçültme simülasyonu ve oyun girdisini
duraklatıyor. Ses mevcut pause politikasını kullanıyor. Odak dönünce girdi
geçmişi eşitleniyor ve varsa manuel pause açık kalıyor. GLFW ve mevcut
Win32 başlangıç yolları bu durumu oyun döngüsüne iletiyor.

### UI-03 — Restore Defaults onayı ve hizmet blip saydamlığı

Ses, görüntü, grafik ve kontrol ayarlarından Restore Defaults seçildiğinde
İngilizce onay sayfası açılıyor. İlk seçim No. Yes yalnızca ilgili sayfayı
sıfırlıyor. No ve geri tuşu ayarlara dönüyor.

Keşfedilmiş hizmet simgeleri görev blip'leriyle aynı radar mesafesi ve alfa
hesabını kullanıyor. Minimap menzil dışındaki simge kenara sabitleniyor.
Hız, ortak radar menzili üzerinden mesafe ve alfayı etkiliyor. Tam ekran
haritada simgeler opak. Oyun doğrulaması bekliyor.

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

## Kalan iş sırası

1. SAV-01 → SAV-02 → SAV-03: F5 konumu, görev sonrası evde doğru doğma ve kayıt kısıtlamaları.
2. VEH-01: kırık camdan geçen atışın sürücüye hasar vermemesi.
3. DIST-01 → DIST-02: 1× üzerindeki araç ve yaya spawn bozulması.
4. WIN-02: odak kaybında pause ve odak geri dönüşü.

**Kullanıcı doğrulaması (19 Kasım 2026):** Kullanıcı son özetteki altı
başlığın çözüldüğünü bildirdi. GPS-01/02, INPUT-02, INPUT-03, INPUT-04,
UI-01 ve CAP-01 kapatıldı. CAP-01 onayı güncel Fullscreen yolundaki
Alt-Tab, Discord ve Ambilight kontrollerini kapsıyor.

**Doğrulama bekleyenler:** MAP-01–07 için son harita değişikliklerinin
derleme ve oyun içi doğrulama durumları yukarıdaki tabloda tutuluyor.

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
çatı Z hesabı değiştirilerek tam çözüm sağlandığı varsayılmamalı. Bu ölçüler
SAV-02 kaynak değişikliğinde kayıt bölgesini seçmek için kullanıldı.

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

### v1.9 derleme sonucu

Kaynak b8cd1c96, GitHub Actions run 44451935119. OpenGL x64
derlemesi ve mevcut birim kontrolleri geçti. Çerçevesiz DXGI yolunda Alt-Tab,
açılış görüntüsü ve yakalama oyun içinde doğrulanmayı bekliyor.

### v1.9 kullanıcı onayı ve ilk ana Fullscreen bağlantısı

Kullanıcı Fullscreen ve Borderless denemelerinde Discord ve Ambilight
sonuçlarını başarılı buldu ve Alt-Tab konusunda mevcut durumu kabul etti.
Bu onay v1.9 denemesine ait.

Windows OpenGL GLFW yolunda Graphics → Screen Format → Fullscreen artık
çerçevesiz DXGI sunumunu doğrudan seçiyor. Enhanced Options içindeki
deney seçeneği kaldırıldı. Eski INI dosyalarındaki `ScreenFormat` okunmaya
devam ediyor. Eski `FullscreenDXGI` anahtarı artık kullanılmıyor.
DXGI başarısız olursa pencere native OpenGL ile sunuluyor.
`-safe-display` Borderless kurtarmasını, `-dxgi-exclusive-fullscreen`
ise önceki exclusive DXGI yolunu seçiyor. Menü bağlantısı ve kaydedilmiş
ayarlarla açılış kaynakta kontrol edildi. Derleme ve oyun içi doğrulama bekliyor.

### Ana Fullscreen sonrası Alt-Tab ve imleç geri bildirimi

Kullanıcı ana Fullscreen'de Alt-Tab sonrası pencerenin arkada kaldığını
ve imlecin Borderless ile aynı gecikmeyi hissettirdiğini bildirdi.
v1.9'da deney seçeneği açıkken parametresiz başlatma gerçek exclusive
DXGI kullanıyordu. Deney seçeneği ana Fullscreen'e taşınırken varsayılan
çerçevesiz DXGI olarak değişti. Bu yol monitörü kaplayan bir pencere
kullanıyor ve exclusive odak geçişine girmiyor. Bu fark pencere
davranışını açıklıyor. Gecikme üzerindeki etkisi henüz ölçülmedi.

Ana Fullscreen'in varsayılanı kaynakta exclusive DXGI'ye döndürüldü.
Ayrı menü seçeneği yok. `-dxgi-composed-fullscreen` ile çerçevesiz DXGI
karşılaştırılabilir. Aynı framebuffer, FPS ve VSync ile exclusive
(`active fullscreen=1`) ve çerçevesiz DXGI (`composed=1`) günlükleri ve
imleç tepkisi karşılaştırılmalı. Exclusive yolun daha önce bildirilen
Alt-Tab beklemesi ve Discord/Ambilight sonucu yeniden gözlenmeli.
Bu değişiklik v1.11 OpenGL derlemesine dahil. Oyun testi bekliyor.

### v1.13 OpenGL derleme sonucu

Kaynak 81352858, GitHub Actions run 44462941598. OpenGL x64 derlemesi
ve üç mevcut birim kontrolü geçti. SAV-01/02/03, DIST-01/02 ve WIN-02
oyun içi doğrulama bekliyor. DIST denemesinde aynı rota ve Density
değerleriyle 1×, 2× ve 3× karşılaştırılmalı. Bölge, mevcut varlıklar
ve havuz kapasitesi gözlenen araç ve yaya sayısını etkiler.
