# v1.3 OpenGL yakalama ve açılış imleci raporu

Tarih: 19 Kasım 2026. Kapsam: kullanıcı gözlemleri, mevcut kaynak incelemesi,
oyun klasöründen salt okunur alınan pencere günlüğü ve VideoMode ayarları.
Bu rapor hazırlanırken kod, sürücü ayarı veya yayımlanmış paket değiştirilmedi;
build/test/workflow çalıştırılmadı.

## 1. Sonuç

- +1 piksel genişlik denemesi sonrası kullanıcı yakalamanın **şimdilik çözülmüş
  göründüğünü** bildirdi. Uzun süreli/diğer sistemlerde doğrulama değildir.
- Borderless'ta kapatılıp yeniden açılan oyunda imlecin en sola yapışması yeni
  **INPUT-02** hatasıdır. Fullscreen → Borderless geçişiyle düzeliyor.
- Kaynakta, bu belirtiyi açıklayabilen **kesin bir tamsayı bölme kusuru** bulundu.
  İlk açılış ve mod değiştirme yollarının boyut güncelleme sırası da farklı.
- +1 piksel pratik bir uyumluluk yöntemi olabilir; mevcut imleç hatası nedeniyle
  v1.3 henüz sorunsuz/nihai dağıtım sayılmamalı. CAP-01 ve INPUT-02 ayrı takip edilir.
- Gerçek exclusive fullscreen yakalaması mevcut +1 denemesiyle düzeltilmedi.
  OpenGL'i koruyan başka sunum yolları araştırılabilir; basit bir bayrakla kesin
  çözüm veya bütün yakalama uygulamalarında başarı sözü verilemez.

## 2. Denemelerin kronolojisi

| Durum | Bildirilen sonuç |
| --- | --- |
| D3D9 | Discord önizlemesinde sorun görülmedi; Ambilight fullscreen'de de çalıştı |
| OpenGL normal pencere | Discord yakaladı |
| OpenGL fullscreen / eski borderless | Discord yakalamadı; Ambilight de bu modlarda çalışmadı |
| v1.2, borderless'ı baştan windowed oluşturma | Kullanıcı sorunun devam ettiğini bildirdi |
| v1.3, borderless genişliği +1 | Kullanıcı şimdilik çözülmüş göründüğünü bildirdi |
| v1.3, borderless kayıtlıyken yeniden açma | İmleç sola kilitleniyor; Fullscreen → Borderless ile düzeliyor |

Son olumlu bildirim Discord/Ambilight için ayrı ayrı ayrıntılandırılmadı.
Ambilight'in normal penceredeki sonucu da ayrıca bildirilmiş değil.
Yeni imleç hatasının kamera, tıklama veya yalnızca menü üzerindeki tüm etkileri
henüz ayrıştırılmadı; kullanıcının bildirdiği kesin belirti imleç konumudur.

## 3. Gerçek pencere ölçüleri

Kanıtlar: [çalışma günlüğü](feedback/2026-11-19/v1.3-display-log.txt) ve
[VideoMode bölümü](feedback/2026-11-19/v1.3-video-mode.txt).
Tam ini veya oyun kaydı kopyalanmadı; yalnızca ilgili ayar bölümü saklandı.

| Alan | Fullscreen günlüğü | Borderless günlüğü |
| --- | --- | --- |
| format | 0 | 2 |
| probe | 1 — ayar açık, bu modda geometriye uygulanmıyor | 1 |
| monitorAttached | 1 | 0 |
| window | 1920×1080 | 1921×1080 |
| framebuffer | 1920×1080 | 1921×1080 |
| pos | 0,0 | 0,0 |

Ini'de Width=1920, Height=1080, Windowed=1, ScreenFormat=2,
BorderlessCaptureProbe=1 görülüyor. Böylece pencerenin gerçekten +1 genişlediği
ve GLFW açısından borderless'ın windowed olduğu doğrulanıyor. Bu kayıt
sürücünün kompozisyon/sunum kararını veya capture API'sini göstermiyor.
Ini'deki Width, çalışma anındaki `RsGlobal.maximumWidth` ölçümü değildir.

## 4. INPUT-02 — tekrar üretim ve beklenen davranış

Kullanıcının tarif ettiği sıra:

1. Screen Format=Borderless ile oyunu kapat.
2. Oyunu tekrar aç.
3. İmleç en sola yapışmış durumda kalıyor.
4. Bir kez Fullscreen'e, ardından Borderless'a geçince düzeliyor.

Beklenen: seçilen modla ilk açılışta da imleç tüm alanda çalışmalı; kullanıcıdan
her seferinde mod değiştirmesi beklenmemeli. Sol kenar gerçek bir koordinattır;
giriş sisteminde hatalı/eksik veri gibi değerlendirilmemeli. Menü ve oyun içi
göreli fare davranışları ayrı korunmalı.

## 5. Kaynak bulgusu — koordinat oranı sıfıra düşebiliyor

`src/skel/glfw/glfw.cpp::cursorCB` içinde şu ifadeler var:

```cpp
xpos * (RsGlobal.maximumWidth / winw)
ypos * (RsGlobal.maximumHeight / winh)
```

Parantez içindeki bölme tamsayılarla yapılıyor. `xpos` double olsa bile bölme
**önceden** tamsayı olarak sonuçlanıyor. Örneğin render genişliği 1920,
pencere genişliği 1921 ise `1920 / 1921 == 0`; bütün X hareketi 0'a çarpılıyor.
`Frontend.cpp` geçici X'i gerçek menü imleci konumuna kopyaladığı için imleç
solda kalabiliyor. Tam ekran boyutları aynıyken oran 1 olduğundan hata gizleniyor.

Bu bir kaynak kusurudur; +1 piksel doğrudan fareyi kilitleyen bir GLFW kuralı
değil, mevcut kusuru görünür hâle getiren koşul olabilir. Başka pencere/renderer
boyut farklarında ve DPI durumlarında da aynı hesap yanlış sonuç üretebilir.

### Açılış ve mod değiştirme sırasının farkı

- İlk açılışta `psPostRWinit` içinden borderless boyutu ayarlanıyor; ana akışta
  `RwInitialised=TRUE` bundan daha sonra atanıyor.
- `resizeCB`, `RsGlobal.maximumWidth/Height` ve kamera boyutunu yalnızca
  `RwInitialised` doğruyken güncelliyor.
- Mod değiştirme yolunda `_psSetVideoMode`, `psPostRWinit` öncesinde
  `RwInitialised=TRUE` yapıyor. Bu kez boyut güncellemesi farklı gerçekleşebiliyor.

Bu fark, soğuk açılışta 1920 render / 1921 pencere, mod değişiminden sonra ise
eşleşen boyutlar oluşmasını açıklayan güçlü bir neden zinciridir. Mevcut logda
`RsGlobal` değerleri olmadığından hatalı karedeki bu ikili henüz doğrudan ölçülmedi.
Kesin tamsayı bölme kusuru ile bu çalışma anı çıkarımı birbirinden ayrılmalıdır.

### Eşlik eden giriş kontrolleri

- `Pad.cpp::UpdateMouse`, GLFW'de `xpos==0` iken erken dönüyor. Sol kenar geçerli
  olduğu için bu kontrol ayrıca incelenecek; menü cursorCB hesabının yerine geçmez.
- `cursorIsInWindow` başlangıçta false; enter callback'iyle güncelleniyor.
- Mouse başlangıcı, son mouse konumu ve HIDDEN/DISABLED geçişleri final pencere
  boyutuyla tutarlı olmalı. Boyut, focus ve cursor-enter durumları karıştırılmamalı.

## 6. Önerilen düzeltme kapsamı — henüz uygulanmadı

1. Koordinat oranını bölmeden **önce** kayan noktaya çevir; sıfır/genişlik ve
   yükseklik geçersizken güvenli çıkış sağla. Window coords ile framebuffer/render
   coords için açık bir dönüşüm kullan; hepsinin aynı ölçekte olduğunu varsayma.
2. İlk açılış ve mod değiştirmeyi, son framebuffer/kamera boyutu ve input
   başlangıcı aynı sırayla tamamlanacak şekilde eşitle.
3. Cursor modu, lastMousePos ve menü imleci başlangıcını final boyutlar/focus
   geçerli olduktan sonra senkronize et; suni delta/tıklama üretme.
4. Geçerli `xpos==0` durumunu ve cursor-enter başlangıcını ayrı düzelt.
5. Gerekirse loga render genişliği/yüksekliği, cursor modu ve focus durumunu ekle.
   Her kare log yazma veya sürücü profili değiştirme çözüm olarak kullanılmayacak.

## 7. +1 piksel yöntemi ne kadar sağlıklı?

Yöntem desteklenen pencere boyutlandırma API'sini kullanır; oyun belleğine
sabit adresle müdahale etmez. Bu değişikliğin kendisi kayıt/oyun fiziği kodunu
değiştirmiyor. Ancak tam monitör boyutundan sapmak aşağıdaki etkileri doğurabilir:

- Sağdaki bir sütunun kırpılması veya komşu monitöre taşması.
- Pencere/framebuffer/render boyutları farklı ele alınırsa görüntü oranı ve
  imleç/tıklama hataları; yeni INPUT-02 bu alanın düzeltilmesi gerektiğini gösteriyor.
- Yakalamanın çalışmasına eşlik eden kompozisyon değişimi varsa bunun frame
  pacing/gecikme etkisi ayrıca ölçülmeli. Daha hızlı/yavaş olduğu ölçülmedi.

1920×1080 örneğinde ek drawable sütunu 1080 piksel, yaklaşık %0,052 ek piksel
alanıdır. Bu yalnızca geometrik hesap; gerçek performans maliyetinin ölçümü değildir.

GLFW destek forumunda benzer yakalama vakasının ±1 pencere boyutuyla düzeldiği
bildirilmiş olması yöntemi anlamlı bir uyumluluk seçeneği yapar; bütün GPU,
monitör düzenleri ve yakalama uygulamalarına garanti vermez:
[GLFW vaka kaydı](https://discourse.glfw.org/t/cannot-view-window-via-screenshot-or-remote-screen-access/2986).

Karar: +1 yöntemi şimdilik korunabilir; önce input/başlangıç tutarlılığı
düzeltilmeli. Kullanıcının şu anki başarısı “geçici olumlu kullanıcı sonucu”
olarak tutulur; her ortam için kalıcı ve tamamen hatasız çözüm diye sunulmaz.

## 8. Normal exclusive fullscreen'de yakalama mümkün mü?

**Teknik olarak araştırılabilir; şu an çalışan bir çözümümüz yok.** Mevcut
fullScreen modu `glfwCreateWindow(..., monitor, ...)` ve OpenGL swap yoluyla
çalışıyor. Borderless pencereye eklenen 1 piksel bu modu etkilemiyor.

| Yol | Ne sağlar? | Sınırı |
| --- | --- | --- |
| +1 borderless + doğru input | Şu an kullanıcıda olumlu capture sonucu; küçük kapsamlı düzeltme | Gerçek exclusive fullscreen çözümü değildir |
| Tam boy native OpenGL sunumunu incelemek | Renderer değişmeden OS/driver sunum etkileşimi araştırılabilir | Her capture uygulamasının izleyeceği garantili basit bir DWM/GLFW bayrağı bulunmuş değil |
| OpenGL çizim + uygulamanın yönettiği DXGI sunumu | GL çizimi korunur; Windows'taki son sunum yolu değişir; gerçek fullscreen tasarlanabilir | Ayrı geliştirme/prototip, donanım/extension kontrolü, senkronizasyon, resize ve cihaz kaybı yönetimi gerekir |

Son seçeneğin teknik dayanağı: `WGL_NV_DX_interop2`, OpenGL ile D3D10/11
kaynaklarının paylaşımını ve GL ile çizilip DirectX üzerinden sunulmasını
tanımlar. Destek varlığı kontrol edilmeli; tüm GPU/sürücülerde varsayılamaz.
[Khronos/NVIDIA uzantı tanımı](https://registry.khronos.org/OpenGL/extensions/NV/WGL_NV_DX_interop2.txt)

Microsoft Desktop Duplication dokümanı API'nin uygulamanın grafik API'sinden
bağımsız olduğunu ve fullscreen DirectX içeriğin de kopyalanabildiğini belirtir.
Bu, Discord/Ambilight'in bizim seçtiğimiz yolu kesin yakalayacağı anlamına gelmez.
[Microsoft Desktop Duplication](https://learn.microsoft.com/en-us/windows-hardware/drivers/display/desktop-duplication-api)

DXGI'nin gerçek fullscreen geçişi ve resize/Alt-Tab davranışı ayrıca tasarlanmalı:
[IDXGISwapChain::SetFullscreenState](https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-setfullscreenstate).
Bu yolu seçmek oyunu D3D9 renderer'ına çevirmek değildir. Windows'a özel sunum
adaptörü, diğer platformların OpenGL yolunu bozmadan ayrılabilir. Her kare CPU
readback/kopya yapan bir kestirme performansı doğrulanmadan ürün çözümü sayılmayacak.

**Sürücü ayarı, registry/global profil değişikliği veya kullanıcıya harici ayar
yaptırmak çözüm planında yok. Fullscreen seçeneğini gizlice borderless olarak
uygulayıp exclusive sorununu çözülmüş göstermeyeceğiz.**

## 9. Sonraki çalışma sırası ve kabul koşulları

Önce INPUT-02: +1 ile olumlu capture sonucu korunurken başlangıç imleci düzeltilmeli.
Ardından SAV-01/SAV-02 ve diğer kayıtlı hatalara dönülür. Exclusive fullscreen
sunum araştırması bu küçük input düzeltmesinden ayrı kapsamdır.

Kullanıcı build istediğinde tek paketle kontrol edilecekler:

- Borderless'ta kapat/aç; menü imleci, sol/sağ kenar ve tıklama hizası.
- Menüden oyuna/geri geçişte göreli kamera girdisi; suni sıçrama olmaması.
- Aynı EXE ile probe=0 ve probe=1 karşılaştırması; normal Windowed ve Fullscreen.
- Discord ve Ambilight sonuçları ayrı; oyun odakta/odak dışında.
- Farklı DPI, Alt-Tab ve varsa komşu monitörde taşma/kırpılma.

Yeni build izni verilene kadar kaynak incelemesi/düzeltme kayıtları biriktirilecek;
bu raporla birlikte build veya test başlatılmadı.
