# CAP-01 — OpenGL ekran yakalama incelemesi

Durum: **kaynakta aday değişiklik hazır; derlenmedi, oyun/yakalama testi yapılmadı**.
Kesin kök neden veya çözülme iddiası yok. Son kullanıcı kararı: OpenGL ana yol.

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

**Sürücü ayarı uygulanmadı.** İncelemede profil seçeneği mevcutsa yalnızca re3
uygulama profili kapsamında, önceki değer kaydedilerek ve geri alınabilir şekilde
değerlendirilmeli. Kontrolsüz global ayar/registry değişikliği yapılmayacak.

## Sonraki doğrulama — kullanıcı build istediğinde

1. Aynı OpenGL EXE, aynı grafik ayarı ve aynı Discord ekran paylaşımıyla normal
   pencere, borderless ve gerçek fullscreen sonuçlarını ayrı kaydet.
2. Borderless'ta ilk oluşturulan RW modunun pencere olduğunu ve GLFW'nin son
   monitor değerinin nil olduğunu doğrula; oyun odağı hem açık hem kapalı denensin.
3. İlk aday yetmezse NVIDIA uygulama profili sunum yöntemini incele. Tek değişken
   değiştir; Discord ve Ambilight sonuçlarını ayrı değerlendir.
4. Gerekirse gerçek GL renderer/GPU, ekran çıkışı, Discord yakalama yolu ve sürücü
   kompozisyon davranışını izle. Tek GPU envanteriyle GPU eşleşmesi kapatılmaz.
5. Pencere çözünürlüğü, Alt-Tab, borderless kapatma ve diğer platformların GLFW
   akışı için regresyon kontrolü yap. Kullanıcı sonucu olmadan CAP-01'i kapatma.

Başvuru: [GLFW pencere/monitor belgeleri](https://www.glfw.org/docs/3.3/window_guide.html#window_monitor).
Monitor değiştirmek GL context'ini yeniden oluşturmaz. Bu yüzden sonradan
kenarlık kaldırmak bütün sunum/yakalama sorunlarının çözümü sayılmaz.

Bu aşamada build, test paketi, sürücü ayarı değişikliği veya CI başlatan push yapılmadı.
