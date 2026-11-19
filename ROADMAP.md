# re3 Enhanced — güncel roadmap

Güncelleme: 19 Kasım 2026. Son yayımlanmış tam paket: **v1.1**.

Öncelik, yeni özelliklerden önce v1.1 geri bildirimlerinin düzeltilmesidir.
Ayrıntılı belirtiler, ekran görüntüleri, beklenen davranış ve kabul ölçütleri
[docs/BUGS.md](docs/BUGS.md) içinde tutulur. Kodlanmış olmak, oyun içinde
çalıştığının doğrulanması anlamına gelmez.

## Çalışma biçimi

- Kullanıcı **“build al”** demeden build, test paketi, workflow dispatch veya
  otomatik CI başlatan push yapılmaz. Düzeltmeler kaynakta biriktirilir.
- Önce maddeler tek tek ele alınır; durumları BUGS.md üzerinde güncellenir.
- Sonraki tam sürümün numarası paketleme aşamasında artırılır. v1.1 arşivi değiştirilmez.
- Kullanıcı istediğinde toplu derleme/doğrulama ve tam, sürümlü paket hazırlanır.

## Öncelikli hata/davranış kuyruğu

| Sıra | Kayıtlar | Hedef |
| --- | --- | --- |
| 0 | INPUT-02 / CAP-01 | Yakalama olumlu sonucunu koruyarak Borderless yeniden açılış imlecini düzeltme |
| 1 | SAV-01, SAV-02, SAV-03 | F5 mevcut konum; görev sonrası doğru ev kayıt noktası; kayıt engelleri |
| 2 | VEH-01 | Kırık ön camdan gerçek sürücü/yolcu isabeti |
| 3 | MAP-01, MAP-02, MAP-03 | Tek ev ikonu, keşif/ilerleme, görülen payphone'un korunması |
| 4 | MAP-04, MAP-05 | Sarı MAP şeridinin tamamını kaldırma; Back hizası/kesilmesi |
| 5 | GPS-01, GPS-02 | Elle waypoint'te en üst yol; rota bağlantıları/boşlukları |
| 6 | DIST-01, DIST-02 | Araç ve yaya spawn mesafeleri ile bağımsız ortam nüfusu sınırları |
| 7 | RM-05, WIN-02, UI-01 | OpenGL ana geliştirme yolu, odak kaybında pause, üçlü ekran biçimi |

CAP-01 için kaynakta aday değişiklik var; build/oyun doğrulaması yok.
Önceki hata kuyruğu açık kalır; ayrıntılı durumlar BUGS.md içinde tutulur.
Kayıt ve harita maddeleri ortak nedenlere sahip olabilir; ayrı kabul ölçütleri korunur.

OpenGL ekran yakalama incelemesi (CAP-01), son kullanıcı talebiyle ayrıca
ele alınıyor. [Teşhis ve yapılan aday değişiklik](docs/OPENGL-CAPTURE.md).

## Gelecek özellikler

### RM-01 — düzgün trainer arayüzü

Durum: roadmap / uygulanmadı.

Ctrl+M/debug arayüzünün yerine sağlıklı, düzenli bir trainer ekranı.
Mevcut debug/trainer menüsünün Ctrl+M kısayolu değiştirilecek. Yeni tuş ve
oyun içi/M harita kısayoluyla çakışma ileride değerlendirilecek.
Debug'daki ayarlar da erişilebilir olmalı. Kullanıcının mevcut arayüzde gördüğü
çok hızlı gezinme/ayar değişimi yeniden oluşmamalı. FPS etkisi şu anda yalnızca
bir şüphe; giriş tekrar aralığı ve zaman hesabı incelenmeli. Yeni arayüzün
gezinmesi farklı FPS'lerde tutarlı olacak; açma kısayolu ve kategoriler ayrıca tasarlanacak.

### RM-02 — radyo çarkı

Durum: roadmap / uygulanmadı.

Definitive Edition benzeri radyo wheel arayüzü. Mevcut radyo/istasyon davranışı
ile uyumlu olacak. Açma-kapama girdileri, seçim/iptal, oyun zamanı/ses politikası
ve kontrolcü desteği uygulama aşamasında netleştirilecek; henüz sözleşmesi kesin değil.

### RM-03 — silah çarkı

Durum: roadmap / uygulanmadı.

Definitive Edition benzeri weapon wheel. Oyuncunun gerçekten sahip olduğu
silah/mühimmat ve native silah seçimiyle uyumlu olmalı. Kısayollar, kontrolcü,
seçim/iptal ve yavaşlatma davranışı tasarım aşamasında kararlaştırılacak.

### RM-04 — doğrudan sayısal fare hassasiyeti

Durum: geliştirme isteği / uygulanmadı.

Fare hassasiyetini doğrudan sayısal değer yazarak ayarlayabilmek. Geçerli aralık, hassasiyet/basamak, ini
saklama biçimi ve kullanıcıya gösterilen birim belirlenecek. Menü imleci ile
oyun içi kamera ayarı karıştırılmayacak. Mevcut değerlerin geçişi korunacak.

### RM-05 — OpenGL / Direct3D 9 mimari kararı

Durum: **Son kullanıcı kararı — OpenGL ana geliştirme yolu.** Önceki D3D9
Windows varsayılanı önerisi değiştirildi. Platformlar arası yapı ve mevcut
OpenGL/librw yolu korunacak; D3D9 alternatif kalacak. Yayımlanmış v1.1
değiştirilmedi; sonraki paketin ana backend'i bu karara göre düzenlenecek.

OpenGL yakalama sorunu CAP-01 olarak açık. Kullanıcı Discord'un normal
pencerede yakaladığını, fullscreen/borderless'ta yakalamadığını netleştirdi.
Kaynak düzeltmesi adayı hazırlandı; derlenmedi/test edilmedi.
[Teşhis notu](docs/OPENGL-CAPTURE.md), [mimari gerekçe](docs/ARCHITECTURE.md).

Kullanıcı farkların, kazanım/kayıpların ve yakalama/Ambilight ilişkisinin
anlatılmasını istiyor. Karşılaştırma ölçülmemiş performans iddialarına dayandırılmayacak.

| Başlık | Şu an bilinen | Açık nokta |
| --- | --- | --- |
| Ambilight | Kullanıcı D3D9 fullscreen'de çalıştığını; OpenGL fullscreen/borderless'ta çalışmadığını bildirdi | GPU/uygulama yakalama yolundaki kesin neden bilinmiyor |
| Discord | D3D9 önizleme sorunsuz; OpenGL normal pencere yakalanıyor, fullscreen/borderless yakalanmıyor | Kesin neden ve gerçek uygulama GPU eşleşmesi hâlâ açık |
| Fare | Kullanıcı iki backend arasında hız farkı bildirdi | Delta, DPI, FPS, kamera/menü ayrımı incelenecek |
| Pencere kodu | D3D9 native Windows; OpenGL GLFW entegrasyonu kullanıyor | Pause, Alt-Tab ve üçlü ekran biçimi eşit davranmalı |
| Performans/görüntü | Karşılaştırmalı ölçüm yapılmadı | Aynı sahne/ayar/FPS koşullarında incelenecek; biri daha hızlı diye varsayılmayacak |
| ASI/CLEO | İki x64 yapı için eşleşen PDB dağıtılıyor | Backend seçimi genel x86 mod uyumluluğu sağlamaz; mod bazında doğrulama gerekir |

**Çıkarım, kesin teşhis değil:** OpenGL'de hem fullscreen hem borderless'ın
başarısız, D3D9 fullscreen'in başarılı olması, sorunun yalnızca borderless
pencere stilinden ibaret olmadığını destekliyor. Grafik API/yakalama yolu ve
sürücü etkileşimi aday nedenler. Bundan “D3D9 her sistemde her yakalama
uygulamasını kesin düzeltir” sonucu çıkarılmayacak.

Yeni input/pencere düzeltmelerinde öncelik OpenGL'dir. Bu tercih FPS/görüntü
kalitesinde ölçülmüş üstünlük veya bütün eski ASI'lerle uyumluluk iddiası değildir.

### RM-06 — Skip Intros ayarı

Durum: özellik isteği / arayüzü uygulanmadı; hata kaydı değildir.

Kullanıcı D3D9'da Rockstar ve GTA açılış videolarının oynadığını, OpenGL'de
oynamadığını bildirdi. Kaynakta da Windows yolundaki `PlayMovieInWindow`
çağrıları etkin; GLFW yolundakiler yorum satırında ve durumlar ilerletiliyor.
Bu, OpenGL'in genel olarak video oynatamayacağı anlamına gelmez.

İstek: oyun ayarlarına **Skip Intros: ON/OFF** eklemek. Yalnızca Rockstar/GTA
açılış videolarını etkiler; görev sinematiklerini etkilemez. Tercih saklanır ve
sonraki açılışta uygulanır. Video yoksa/oynatılamıyorsa açılış akışı takılmamalı.
Varsayılan değer henüz seçilmedi.

Mevcut altyapı: `gbNoMovies`, `[General] NoMovies` ile zaten okunup kaydediliyor
(`re3.cpp`); Windows açılış akışı bu bayrağı kullanıyor. Yeni, çakışan bir
bayrak yerine bu altyapıya UI bağlanması incelenecek. OpenGL alternatifinde
video desteği eklenmeden OFF seçeneği videolar çalışıyormuş gibi sunulmamalı.

## v1.1 doğrulama geçmişi ve korunacak kontroller

- İki x64 backend derlendi; önceki birim testleri, EXE/PDB eşleşmeleri ve ZIP
  bütünlüğü doğrulandı. Bunlar yeni hata kayıtlarını kapatmaz.
- Kullanıcı manuel kayıtta can doldurmayı doğruladı; korunacak.
- Kayıtta doğru ada seçiliyor; çatıda doğma ve F5 konum politikası hâlâ açık.
- Ambilight gözlemi yukarıdaki RM-05'te. Discord için kullanıcı D3D9 yayın
  önizlemesini kontrol etti ve şu an sorun görmediğini bildirdi; bu olumlu sonuç
  yalnızca D3D9'a aittir ve uzun süreli yayın sonucu olarak genellenmez.
  Sonraki kullanıcı testi OpenGL'de Discord'un çalışmadığını doğrulayan başarısız
  sonuç olarak kaydedildi; OpenGL yakalama sorunu açık kalır.
- Henüz sonucu bildirilmeyen önceki kontroller: hız/T ikon ölçeği, GPS renkleri
  ve hareketli hedefler, kapalı köprü/tek yön rotaları, görev-sonu menünün tek
  tetiklenmesi ve iptali, sekiz kayıt slotu/mission retry, sağlık kaydı/başarısız
  save geri alma, 16:9–4:3 arayüz, çoklu monitör/Alt-Tab ve x64 ASI/CLEO açılışı.
- First person ve ragdoll önceki kullanıcı isteğiyle kaldırılmış durumda kalır.
- Çok hedefli görevlerin tam görev mantığını anlama hâlâ GPS kapsamı dışında;
  renk ve gerçek hedef yüksekliği davranışı korunmalı.

## Kullanıcı onaylı dar kapsamlı derleme

v1.2: yalnızca OpenGL; CAP-01 başlangıç adayı + UI-01 Screen Format üçüncü
seçeneği. Kullanıcı bu iki değişikliği denemek için build istedi. Sürücü
ayarlarına müdahale yok. Diğer hata kayıtları açık kalır.

Derleme sonucu: v1.2 için tek OpenGL işi ve mevcut birim kontrolleri geçti.
CAP-01/UI-01 oyun içi doğrulaması bekleniyor. Sürücü ayarı değiştirmek dağıtım
çözümü olarak reddedildi; düzeltme oyun kodunda yapılacak. Diğer hata kayıtları açık.

Son geri bildirim: kullanıcı v1.2 yakalama adayının çözmediğini bildirdi. CAP-01
kapanmadı. Sıradaki öneri, fullscreen'e otomatik yükseltilme hipotezini 1 piksel
farklı pencere boyutuyla sınamak; henüz kodlanmadı/build alınmadı. Büyük ölçekli
uygulama-içi DXGI sunumu yalnızca sonraki tasarım seçeneği olarak kaydedildi.

## v1.3 OpenGL yakalama probu

Kullanıcı +1 piksel denemesini istedi. Windows Borderless genişliği +1,
uygulama içi karşılaştırma bayrağı ve başlangıç/mod değişimi ölçü günlüğü
hazırlandı. Yalnızca OpenGL build/paket planlandı. Gerçek capture sonucu henüz yok.

## v1.3 kullanıcı sonucu / INPUT-02

+1 Borderless yakalama şimdilik olumlu, fakat kayıtlı Borderless modunda yeniden
açılışta imleç sola kilitleniyor; Fullscreen → Borderless geçişi düzeltiyor.
Kaynakta tamsayı koordinat oranı ve başlangıç senkronizasyonu incelendi. Önce
bu input blokajı, sonra kayıt hataları. Gerçek exclusive fullscreen yakalaması
ayrı tasarım işi olarak açık. [Tam rapor](docs/DISPLAY-REPORT-v1.3.md).
Bu adım yalnızca raporlama; yeni kod/build/test/push yapılmadı.

## Ayarlar menüsünün sırasını yeniden düzenleme

Kullanıcı ayarlar menüsündeki seçeneklerin sırasını karmaşık buldu ve
düzenlenmesini istedi. Yeni sıralama ve gruplama kullanıcı tarafından daha
sonra tarif edilecek. O tarif gelene kadar mevcut arayüzde değişiklik yapılmayacak.
Kapsam menü düzenidir; ayarların davranışını değiştirme isteği olarak
yorumlanmamalı.
