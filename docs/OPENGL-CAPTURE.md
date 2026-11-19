# CAP-01 — OpenGL ekran yakalama incelemesi

Durum: **kullanıcı v1.2 adayının sorunu çözmediğini bildirdi; CAP-01 açık**.
Kesin kök neden veya çözülme iddiası yok. Son kullanıcı kararı: OpenGL ana yol.

## v1.2 sonrası sonuç ve sıradaki aday

Kullanıcının son bildirimi: “yok olmadı halen”. Başlangıcı windowed yapmak
tek başına çözüm olmadı. Bu geri bildirimde Discord ve Ambilight'in son durumları
ayrı ayrı belirtilmedi; iki uygulama için yeni ayrı başarı sonucu varsayılmadı.

**Sıradaki önerilen kontrollü deneme:** monitörü tam kaplayan borderless pencere
yerine, pencere boyutunu tek yönde 1 piksel farklı yaparak yakalamanın geri gelip
gelmediğini gözlemek. Windows/OpenGL sürücüsünün tam ekran boyutundaki pencereyi
kompozisyon dışına yükseltmesi bir hipotezdir; v1.2'de gerçekten böyle olduğu ölçülmedi.

GLFW'nin kendi destek forumunda benzer bir yakalama vakasında aynı olasılık
tartışılmış ve uygulama yazarı ekran genişliğinden 1 piksel eksik/fazla pencereyle
sorunun düzeldiğini bildirmiştir:
[GLFW — Cannot view window via screenshot or remote screen access](https://discourse.glfw.org/t/cannot-view-window-via-screenshot-or-remote-screen-access/2986).
Bu bir vaka raporudur, Windows/GLFW garantisi değildir.

- Önce aynı EXE'de normal boyut ve 1 piksel farklı boyut karşılaştırılmalı;
  başka ayar aynı anda değiştirilmemeli.
- +1 piksel dışarı taşırma görünür boş çizgi bırakmayabilir, fakat kırpılma,
  çoklu monitörde taşma, framebuffer/aspect ve imleç koordinatları değerlendirilir.
  -1 piksel ise masaüstünde görünür ince boşluk bırakabilir.
- Bu adım teşhis/uyumluluk adayıdır; piksel ölçüsü değişmeden kusursuz borderless
  sağlandığı iddia edilemez. v1.3 için kaynakta +1 genişlik denemesi hazırlandı.
- Çalışırsa sorun genel OpenGL çiziminden çok fullscreen sunum/kompozisyon
  sınırında daraltılmış olur. Çalışmazsa bu hipotez doğrulanmış sayılmaz.

Daha büyük uygulama-içi seçenek: Windows'ta OpenGL ile çizilen son görüntüyü
uygulamanın yönettiği DXGI sunum yoluna aktarmak. Bu, D3D9 renderer'ına geçmek
değildir; ancak GPU paylaşımı/interop, senkronizasyon, resize, cihaz kaybı ve
desteklenmeyen sürücüler için fallback tasarımı gerektirir. Bu aşamada uygulanmadı,
performansı veya capture uyumu doğrulanmadı; basit pencere ayarı gibi sunulmayacak.

Sürücü ayarlarını değiştirmek veya kullanıcıya harici ayar önermek çözüm planına dahil değildir.

## Bilinenler

- Discord bütün ekran paylaşımı: normal OpenGL penceresi yakalanıyor;
  fullscreen ve borderless yakalanmıyor (kullanıcı netleştirmesi).
- D3D9 Discord önizlemesi kullanıcıda sorunsuz görünüyor.
- Ambilight OpenGL fullscreen/borderless'ta çalışmıyor, D3D9'da çalışıyor.
  Ambilight'in normal OpenGL pencere sonucu ayrıca bildirilmedi.
- Salt okunur Windows sorgusu: NVIDIA GeForce RTX 3060, sürücü 32.0.16.1692;
  o anda 1920×1080 görüntü modu. Sorgu yalnızca bu ekran kartını listeledi.
- Bu envanter, oyunun/Discord'un gerçekten kullandığı GPU bağlamını kanıtlamaz.
  Windows uygulama GPU tercihleri sorgusu re3/Discord için açık eşleme döndürmedi;
  NVIDIA uygulama profili ayrıca okunmuş/değiştirilmiş değil.

## Kaynakta bulunan fark ve aday düzeltme

`src/skel/glfw/glfw.cpp::psSelectDevice`, Borderless açık olsa bile alttaki
Windowed tercihi kapalıysa önce exclusive RenderWare video modunu seçiyordu.
librw, bu mod bayrağına bakarak `glfwCreateWindow(..., monitor, ...)` çağırıyor.
Daha sonra `psPostRWinit` içindeki `ApplyWindowMode`, aynı pencere/context'i
`glfwSetWindowMonitor(..., nil, ...)` ile borderless pencereye çeviriyordu.

**Yapılan aday değişiklik:** Borderless açıkken, GL penceresi oluşturulmadan
önce pencere modu seçiliyor. Böylece başlangıçtaki gereksiz exclusive geçiş
kaldırılıyor. `PSGLOBAL(fullScreen)` buna uygun ayarlandı. Borderless kapatılırsa
alttaki fullscreen/windowed tercihini geri yükleme bilgisi korunuyor.

Bu, gereksiz bir başlangıç geçişini düzeltiyor; sürücünün yakalama davranışını
kesin düzelttiği henüz bilinmiyor. Exclusive fullscreen yakalama sorununu tek
başına çözdüğü iddia edilmiyor. Gerçek fullscreen ile borderless sonuçları
sonraki doğrulamada ayrı değerlendirilmeli.

## NVIDIA sunum yöntemi — ikinci araştırma yolu

Sunshine'ın kendi belgeleri, Windows/NVIDIA'da fullscreen OpenGL/Vulkan
yakalamanın sunum yolundan etkilenebildiğini ve **Vulkan/OpenGL present method →
Prefer layered on DXGI Swapchain** seçeneğini kullandığını açıklar:
[Sunshine resmi yapılandırma belgesi](https://docs.lizardbyte.dev/projects/sunshine/latest/md_docs_2configuration.html#nvenc_opengl_vulkan_on_dxgi).

Bu, RTX 3060 ve normal pencere/fullscreen ayrımıyla ilgili güçlü bir araştırma
adayını gösterir; Discord/Ambilight'in aynı API'yi kullandığını veya aynı kök
nedene sahip olduğunu kanıtlamaz. Kullanıcı Sunshine kullanıyor diye varsayılmadı.
DXGI üzerinden sunum, oyunun renderer'ını D3D9'a çevirmek değildir.

**Kullanıcı kararı:** Sürücü ayarı değiştirmek/önermek dağıtılan ürünün çözümü
olarak kabul edilmiyor. Profil veya global ayar değiştirilmeyecek. Yukarıdaki
NVIDIA bulgusu yalnızca sunum yolu araştırmasına referanstır; kurulum adımı değildir.

## Sonraki doğrulama — v1.2 kullanıcı denemesi

1. Aynı OpenGL EXE, aynı grafik ayarı ve aynı Discord ekran paylaşımıyla normal
   pencere, borderless ve gerçek fullscreen sonuçlarını ayrı kaydet.
2. Borderless'ta ilk oluşturulan RW modunun pencere olduğunu ve GLFW'nin son
   monitor değerinin nil olduğunu doğrula; oyun odağı hem açık hem kapalı denensin.
3. İlk aday yetmezse sunum/kompozisyon yolunu uygulama kodunda araştır; kullanıcıdan
   sürücü profili değiştirmesini isteme. Discord ve Ambilight sonuçlarını ayrı değerlendir.
4. Gerekirse gerçek GL renderer/GPU, ekran çıkışı, Discord yakalama yolu ve sürücü
   kompozisyon davranışını izle. Tek GPU envanteriyle GPU eşleşmesi kapatılmaz.
5. Pencere çözünürlüğü, Alt-Tab, borderless kapatma ve diğer platformların GLFW
   akışı için regresyon kontrolü yap. Kullanıcı sonucu olmadan CAP-01'i kapatma.

Başvuru: [GLFW pencere/monitor belgeleri](https://www.glfw.org/docs/3.3/window_guide.html#window_monitor).
Monitor değiştirmek GL context'ini yeniden oluşturmaz. Bu yüzden sonradan
kenarlık kaldırmak bütün sunum/yakalama sorunlarının çözümü sayılmaz.

Kullanıcı bu iki değişiklik için build istedi. Tek OpenGL işi derlendi ve mevcut
birim kontrolleri geçti. Sürücü ayarı değiştirilmedi; D3D9 build başlatılmadı.

## v1.3 — kullanıcı denemeyi istedi

Yalnızca Windows OpenGL Borderless genişliği +1 yapıldı. Diğer pencere
biçimleri ve Linux/macOS geometrisi değiştirilmedi. Uygulama içi ini bayrağı
`[VideoMode] BorderlessCaptureProbe` varsayılan 1; 0 ile aynı EXE
tam genişliğe geri dönebilir (oyunu yeniden başlatmak gerekir).
`re3-display.log` gerçek pencere/framebuffer boyutunu ve GLFW monitor
bağlantısını başlangıç/mod değişiminde kaydeder. Sağ kenardaki taşan sütun,
çoklu monitör, görüntü oranı ve fare koordinatları denemede değerlendirilecek.
Sürücü ayarı yok; yalnızca OpenGL build/paket hazırlanacak. Yakalama sonucu bilinmiyor.
