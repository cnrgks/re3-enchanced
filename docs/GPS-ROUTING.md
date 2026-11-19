# GPS katman ve ara sokak incelemesi

19 Kasım 2026 · `enhanced-source` · Kaynak incelemesi. Son bildirimin konumu,
kullanılan paket ve hedef türü henüz bilinmiyor. Aşağıdaki kod davranışları
son bildirimi açıklayabilecek adaylardır. Oyun içinde yeniden üretim bekliyor.

## Uygulama durumu

Rota araması geçerli başlangıç ve hedef segmentlerini birlikte değerlendiriyor.
Elle waypoint'in seçilen yükseklik katmanı işaret kaldırılana kadar korunuyor.
Açık son bağlantısı olan bir hedefe ulaşılabiliyorsa arama onu tercih ediyor.
Son bağlantı doğrulanamadığında çizgi yol ağında bitiyor. Oyuncu için açık bir
başlangıç bağlantısı bulunamazsa rota, bir sonraki yeniden hesaplamaya kadar
gizli kalıyor. Oyuncu konumu, yol katmanı ve kalan rota mesafesi varışta birlikte
kontrol ediliyor. Açık giriş önceliği ve erişilemeyen açık girişin yol ucuna geri
düşmesi için sentetik regresyon senaryoları eklendi. Derleme ve oyun testi bekliyor.

## Değişiklik öncesi kaynak bulguları

| Belirti | Mevcut davranış | Olası sonuç |
| --- | --- | --- |
| Viyadük, tünel veya paralel sokakta yanlış başlangıç | `Enhanced.cpp::UpdateRoute` tek başlangıç segmenti seçiyor. Önce 40 birim içinde, düşey farkı 4 birimden az ve görüşü açık bir segment arıyor. Bulamazsa 120 birime kadar en yakın 3B segmente geri düşüyor. | Başlangıç duvarın ötesine veya başka kata geçebilir. Seçilen segment çıkmazdaysa başka giriş denenmez. |
| Haritadan konan işaret yanlış yol katına bağlanıyor | Elle hedef 2B izdüşürülüyor. En yakın yol 8 birim içindeyse, ona en fazla 2 birim daha uzak adaylar önce yüksekliğe göre sıralanıyor. Sonraki aramalar katman kilidi olmadan, en yakın yola ek 18 birimlik pencere içinden ilk erişilebilir adayı kabul ediyor. | Üst kat seçimi bir sıralama tercihi olarak kalıyor. Üstteki yol erişilemiyorsa rota sessizce alttaki yola kayabilir. |
| Görev hedefinin arka sokağı, servis yolu veya alt katı seçiliyor | Bilinen Z ile 3B mesafe kullanılıyor, fakat düşey fark ayrı bir sınır olarak uygulanmıyor. Ayrıca `Script3.cpp` bazı koordinat blip'lerinde eksik Z'yi zemin aramasıyla dolduruyor. | Yakın fakat farklı kat/yan sokak seçilebilir. Blip Z'sinin kaynağı bazı görevlerde ayrıca incelenmeli. |
| Ara sokakta rota erken bitiyor, dolanıyor veya görünmüyor | En fazla 10 benzersiz hedef segmenti sırayla deneniyor. İlk erişilebilir aday kazanıyor. Son bağlantının açık olup olmadığı ancak rota seçildikten sonra kontrol ediliyor. Kontrol en fazla 40 birim ve yüklenmiş çarpışma bölgesiyle sınırlı. | Daha iyi bir mahalle girişi daha geride kalabilir. Duvarın öte tarafındaki en yakın yol, açık bir girişe karşı kazanabilir. Uzak hedefte bağlantı henüz doğrulanamamış olabilir. |
| İşarete uğramadan rota veya waypoint kayboluyor | Görev rotası bilinen hedefe 12 birimlik 3B yakınlıkta aramadan önce gizleniyor. Elle waypoint ise seçilen yolun Z'siyle 12 birimlik yakınlık ve 3 birimlik düşey fark sağlandığında, son bağlantı kontrolünden önce silinebiliyor. | Paralel sokaktan geçerken veya bir engelin yanında yanlış varış oluşabilir. |

`RouteGraph::VisitRoadEdges` araç ve yaya ağlarını ayırıyor, SCM ile kapanan
yolları eliyor ve araç bağlantılarında yön kısıtını uyguluyor. Bu kurallar
korunmalı. Yol segmentinin mevcut olması, o segmente oyuncunun bulunduğu
avludan veya binanın yanından geçilebildiğini tek başına göstermez. Uygun
bağlantı kaynak ağda eksikse, aday sıralamasını değiştirmek geçit oluşturmaz.

## Önerilen uygulama sırası

1. **Varışı rotaya bağla.** Mesafeyi ön koşul olarak kullan. Oyuncunun doğru
   katmandaki son erişilebilir segmente ilerlediğini ve harita işaretine kalan
   kısa bağlantının açık olduğunu doğrula. Bağlantı belirsiz veya kapalıysa
   waypoint kalsın. Görev blip'inin yaşam döngüsü SCM'de kalsın.
2. **Önce yol katını ve erişimi seç.** Başlangıçta oyuncunun gerçek Z'sini,
   yakın yol yüzeyini ve kısa bağlantıyı birlikte değerlendir. Benzer adaylarda
   önceki segmenti ve hareket yönünü yardımcı sinyal olarak kullan. Rampada
   bağlı ardışık segmentler arasında geçişe izin ver. Elle işaretlemede mevcut
   üst yol politikasını aynı 2B koridor içindeki üst yol için koru ve seçilen
   katmanı waypoint boyunca sakla. Bilinen Z'li görev hedefini kendi katına
   bağla. Uzak veya Z'si belirsiz hedeflerde yol grafiğini kullan, fiziksel son
   bağlantıyı çarpışma verisi geldiğinde yeniden değerlendir.
3. **Birden fazla geçerli girişi tek yönlü aramada değerlendir.** Sanal
   başlangıçtan uygun segmentlere, uygun bitiş segmentlerinden sanal hedeflere
   bağlan. Önce katman ve erişim koşullarını uygula, ardından yol mesafesiyle
   bağlantı maliyetini birlikte karşılaştır. Seçilen başlangıç/bitiş izdüşümünü,
   aynı segmentteki doğrudan rotayı ve tek yön kuralını koru. Mevcut
   `EnhancedRoute::Search` altyapısıyla bir defada adaylara ulaşılabilir. Yerel
   aday kümesi genişletilecekse bunu aynı katmanda yap. Ulaşılamayan hedefte
   rota çizgisini doğrulanmış son yol noktasında bitir ve waypoint'i göster.
4. **Tekrarlanan konumlarda harita ağını incele.** Rota başına isteğe bağlı tek
   teşhis kaydında hedef türü, araç/yaya modu, oyuncu ve hedef XYZ'si, seçilen
   başlangıç/bitiş segmentleri, katman, bağlantı sonucu, kapanış/yön nedeniyle
   elenen adaylar ve rota maliyeti bulunsun. Geçerli bir sokak grafikte yoksa
   önce o kesimin düğüm/bağlantı verisini doğrula. Yalnızca kanıtlanan yerel
   ağ hatası için veri düzeltmesi düşün. Arama süresi ve incelenen düğüm sayısı
   ölçüldükten sonra gerekirse aday taramasına mekânsal indeks ekle.

## Doğrulama senaryoları

| Senaryo | Kabul ölçütü |
| --- | --- |
| Aynı X/Y'de köprü ve alt yol | İki başlangıç katından da çizgi doğru yola oturur. Elle waypoint üst yolu seçer. Bilinen Z'li görev hedefi kendi katında kalır. |
| Rampa ve tünel ağzı | Rota bağlantıyı takip eder, rampa boyunca paralel yol katlarına sıçramaz. |
| Duvarlı avlu veya yakın arka sokak | Rota gerçek girişten dolaşır. Paralel sokaktan geçiş waypoint'i silmez. |
| Tek yön, çıkmaz sokak, SCM ile kapanan geçit | Yasak bağlantı kullanılmaz. Uygun alternatif giriş bulunur veya işaret erişilemeyen hedef olarak kalır. |
| Yol ağı dışındaki bina ve henüz yüklenmemiş uzak hedef | Çizgi ulaşılabilir yolda biter. Bölge yüklenince son bağlantı yeniden değerlendirilir. |
| Aynı segment, kavşak ve araç/yaya geçişi | İleri yönlü doğrudan rota, zorunlu dolaşma ve ağ değişimi tutarlı çalışır. |

Mevcut `tests/enhanced_route.cpp` yön, kapanış, erişilemeyen hedef ve alternatif
giriş için sentetik arama kapsamı sağlıyor. Katman seçimi, kısmi segmentler ve
yanlış varış için uygulama aşamasında odaklı regresyon senaryoları eklenmeli.
Derleme/test sonuçları ile oyun içi doğrulama ayrı kaydedilmeli. Eski
[rota ekran görüntüsü](feedback/2026-11-19/04-route-gap-and-detour.png) ve
[tünel örneği](feedback/2026-11-19/v1.13-gps-crossing.png) tarihsel referanstır.
