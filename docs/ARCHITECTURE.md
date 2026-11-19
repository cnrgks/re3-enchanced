# Grafik altyapısı kararı — 19 Kasım 2026

**Son kullanıcı kararı: OpenGL ana geliştirme yolu olarak korunacak.**
D3D9 alternatif olarak kalır. Önceki Windows için D3D9 önerisi bu kararla
değiştirildi. Yayımlanmış v1.1 değiştirilmedi. Sonraki paketleme varsayılanı
OpenGL ana / Direct3D9 alternatif olarak güncellendi; iki backend
derleyen CI matrisi değiştirilmedi.
OpenGL borderless başlangıcında kaynak düzeltmesi adayı hazırlandı; derlenmedi.

## Kararın platformlar arası kapsamı

Kullanıcı platformlar arası desteği ve mevcut OpenGL/librw yolunu önceliklendirdi.
Projeyi D3D9-only yapmıyoruz; OpenGL Windows dahil ana geliştirme yoludur.
D3D9 mevcut karşılaştırma/alternatif yol olarak korunur.

- Mevcut `premake5.lua`, Windows için D3D9 ve OpenGL; Linux/BSD için OpenGL
  hedefleri tanımlıyor. Fork README'si Windows/Linux/macOS/FreeBSD desteğini bildiriyor.
- D3D9-only native grafik yolu Windows odaklıdır. OpenGL yolunu kaldırmak diğer
  platformların mevcut native grafik hedefini ortadan kaldırır. Windows'ta D3D9
  EXE dağıtmak tek başına projenin diğer platform kaynaklarını/devamını engellemez.
- Her platforma uygun ayrı executable/derleme gerekir. Mevcut Windows EXE,
  DLL ve ASI paketi Linux/macOS için native dağıtım veya mod uyumluluğu değildir.
- Yeni GPS/kayıt/harita/oynanış özellikleri ortak C++/librw arayüzlerinde tutulmalı;
  doğrudan D3D/Win32 çağrıları ortak oynanış koduna yayılmamalı. Pencere, odak,
  fare ve video işleri platform adaptörlerinde aynı davranış sözleşmesini sağlamalı.
- Bu fork'taki yeni değişiklikler Linux/macOS'ta henüz derlenip doğrulanmadı.
  Çoklu platform desteği, yalnızca OpenGL dosyalarını tutmakla doğrulanmış sayılmaz.

## Özgün GTA III ve uzun vadeli sağlamlık

GTA III'ün temel motoru OpenGL değil **RenderWare**'dir. Fork'un README'si özgün
RenderWare yolunu D3D8, librw yeniden uygulamasını ise D3D9/OpenGL/OpenGL ES
olarak açıklar. Dolayısıyla OpenGL'i korumak “özgün motorla aynı grafik API'sini
korumak” gerekçesine dayanmamalı.

Bir API'nin adı daha az hata garantisi vermez. Kullanıcının kayıt, GPS, cam ve
ikon sorunları doğrudan grafik API'si değiştirilerek çözülmüş sayılmaz. Uzun
vadeli tercih: ortak oyun çekirdeği + sınırlı platform katmanı + ana OpenGL
yolu + alternatif D3D9.
Bedeli iki backend'in pencere/girdi/görsel davranışını koruma yüküdür; karşılığı
platform desteğini kaybetmemektir. Performans/kararlılık üstünlüğü ölçülmeden ilan edilmez.

Kaynaklar: [fork README](../README.md), [platform hedefleri](../premake5.lua),
[GLFW platform desteği](https://www.glfw.org/faq.html),
[Microsoft Direct3D 9](https://learn.microsoft.com/en-us/windows/win32/api/_direct3d9/).

## Gerekçe ve karşılıkları

| Başlık | D3D9 | OpenGL |
| --- | --- | --- |
| Kullanıcının Discord testi | Yayın önizlemesinde şu an sorun görülmedi | Çalışmadığı bildirildi |
| Kullanıcının Ambilight testi | Fullscreen'de de çalışıyor | Fullscreen ve borderless'ta çalışmıyor |
| Açılış videoları | Rockstar/GTA videoları çalışıyor | Mevcut GLFW yolunda video çağrıları kapalı |
| Pencere/girdi entegrasyonu | Native Windows yolu | GLFW yolu; kullanıcı fare hızı farkı bildirdi |
| Bakım odağı | Mevcut Windows kullanımına öncelik verilebilir | İkinci yolun eşdeğer davranışını ayrıca sürdürmek gerekir |
| Taşınabilirlik | Bu backend Windows odaklı | GLFW/OpenGL yolunu korumak diğer platformlar için avantaj sağlar |
| FPS/görüntü kalitesi | Karşılaştırmalı ölçüm yok | Karşılaştırmalı ölçüm yok |

Önceki D3D9 önerisi kullanıcı sisteminde çalışan özelliklere dayanıyordu. D3D9 seçmek yerleşik GPS,
kayıt, harita veya diğer C++ özelliklerinden vazgeçmek değildir. Daha yüksek FPS,
daha iyi görüntü veya orijinal GTA III x86 ASI uyumluluğu sağladığı iddia edilmez.
Güncel kararla ana odak OpenGL; yakalama sorunu ayrıca giderilecek.

## OpenGL'de yakalama neden çalışmıyor?

**Kesin neden henüz bulunmadı.** Kaynakta iki farklı sunum yolu var:
`vendor/librw/src/d3d/d3ddevice.cpp` D3D `Present` kullanıyor;
`vendor/librw/src/gl/gl3device.cpp` GLFW `glfwSwapBuffers` kullanıyor.
Pencereyi borderless yapmak bu grafik yolunu değiştirmiyor.
[GLFW belgeleri](https://www.glfw.org/docs/3.3/window_guide.html#window_monitor)
de pencere/tam ekran değişiminde OpenGL context'inin korunduğunu açıklıyor.

Kullanıcının iki yakalama uygulamasındaki benzer sonucu, OpenGL sunumu ile
sürücü/masaüstü kompozisyonu/yakalama yolunun etkileşimini aday yapıyor. Hangi
yakalama API'sinin gerçekten kullanıldığı, GPU seçimi ve sürücü davranışı
izlenmeden bunlardan biri kesin kök neden olarak gösterilemez. İki uygulamanın
aynı hataya veya aynı yakalama API'sine sahip olduğu da kanıtlanmış değil.

Bu, “Windows OpenGL yakalayamaz” demek değildir. Microsoft, Desktop Duplication'ın
uygulamanın kullandığı grafik API'sinden bağımsız olduğunu belirtir:
[Desktop Duplication](https://learn.microsoft.com/en-us/windows-hardware/drivers/display/desktop-duplication-api).
Bu genel özellik, kullanıcının Discord/Ambilight yolunun hatasız çalıştığını kanıtlamaz.

Güncel ayrım: kullanıcı Discord'un normal OpenGL penceresinde yakaladığını,
fullscreen ve borderless'ta yakalamadığını bildirdi. Ambilight için normal
pencere sonucu ayrıca doğrulanmadı. RTX 3060, Windows sürücüsü
32.0.16.1692 olarak okundu; uygulamaların gerçek render GPU'su henüz
izlenmedi. Araştırma ve kaynak değişikliğinin durumu
[OPENGL-CAPTURE.md](OPENGL-CAPTURE.md) içinde.

## Intro farkının açıklaması ve Skip Intros

`src/skel/win/win.cpp` içindeki Windows yolunda `PlayMovieInWindow` etkin ve
DirectShow/COM üzerinden `movies/Logo.mpg` ile GTA başlık videosunu oynatıyor.
`src/skel/glfw/glfw.cpp` içinde bu çağrılar yorum satırında; intro durumları
video oynatmadan ilerletiliyor. Dolayısıyla fark yalnızca grafik API'sinin
yetenekleriyle açıklanamaz; platform entegrasyonları farklıdır.

`src/core/re3.cpp`, mevcut `gbNoMovies` değerini `[General] NoMovies` olarak
okuyup kaydediyor; Windows açılış akışı bunu kullanıyor. Oyun içi **Skip Intros**
ayarının bu mevcut altyapıyı kullanması planlanır; henüz UI eklenmedi.
Kapsam yalnızca açılış videolarıdır, görev sinematikleri değildir. Ayrıntı RM-06'da.

Build/test veya otomatik CI başlatan push yapılmadı; kullanıcı talebi beklenir.
