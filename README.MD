# Walkie-Talkie Keyer

So, hier isser: Der Walkie-Talkie Keyer. Das Projekt, an dem ich die letzten
Wochen nebenher rumgepusselt habe. Es ist eine komplette Hardware zum Betrieb
eines Handfunkgeräts als CW Station.

## Noch nie gestellte Fragen und ein paar Antworten:


### Was macht das Gerät?

CW über FM.


### Das kann ein Piepser, den ich ans Funkgerät halte. Wieso eine extra Hardware?

Der Walkie-Talkie Keyer übernimmt die komplette Steuerung. Er schaltet beim
ersten Zeichen das Handfunkgerät auf Sendung, generiert lokal einen Mithörton
und gibt die Morsezeichen auf das Funkgerät um wenige 100ms zeitverzögert aus.
Damit hat das Funkgerät Zeit, auf Sendung zu schalten. Mit dem Walkie-Talkie
Keyer ist CW mit einer Handfunke ähnlich komfortabel wie mit einem Gerät, das
CW native unterstützt: Einfach losmorsen. Das Gerät schaltet automatisch auf
Sendung und nach dem letzten Zeichen wieder zurück.


### Wofür braucht man das?

Ich habe es primär gebaut um auf dem Congress TOTA in CW machen zu können,
weil ich dort vermutlich nur ein Handfunkgerät haben werde. Der Walkie-Talkie
Keyer funktioniert aber nicht nur im Amateurfunk, sondern kann auch einfache
PMR Funkgeräte ansteuern und eignet sich deshalb auch zur Ausbildung oder als
Spaßprojekt für Kids. Der Keyer kann im Prinzip auch "große" Funkgeräte
ansteuern, solange diese PTT gegen Masse schalten und dafür keine hohen
Ströme/Spannungen notwendig sind.


### Gab's das nicht schon mal?

Ja, das gab's schon mal. Burkhard John (DK5JG) hat 2006 eine ähnliches Projekt
vorgestellt, den "FM-Keyer". Von diesem Gerät stammen einige der Ideen.
Schaltung, Software und Board des Walkie-Talkie Keyers sind aber komplett neu
und von mir.


### Paddle oder Handtaste?

Der Keyer kann beides, mit Handtaste ist aber keine Konfiguration möglich. Da
alle Einstellungen (bis auf "Handtaste oder Paddle") ausfallsicher gespeichert
werden, sollte das aber kein großes Problem sein. Nachdem die Konfiguration
mit einem Paddle durchgeführt wurde, kann mit genau diesen Einstellungen auch
eine Handtaste verwendet werden.


### Wie wird eine Handtaste erkannt?

Für ein Paddle wird ein 3,5 mm Stereo Klinkenstecker verwendet, für eine
Handtaste derselbe Stecker in Mono. Durch den Mono Stecker wird das rechte
Paddle ("Dah") an Masse gelegt. Das erkennt der Keyer beim Start und schaltet
direkt auf Handtaste um. Alternativ kann das rechte Paddle beim Start gedrückt
werden, oder mit dem "SK" Kommando (siehe unten) von Paddle auf Handtaste
umgeschaltet werden.

Die Einstellung "Handtaste" bleibt erhalten, bis der Keyer stromlos gemacht
wird. Ein Rückschalten durch Kommando ist nicht möglich.


### Wie bediene ich den Keyer?

Der Keyer benötigt externe 5V aus einem USB Port (Handy oder Power Bank). Er
wird auf der einen Seite mit dem Handfunkgerät verbunden. Auf der anderen
Seite wird die Taste angeschlossen. Neben der Einstellung für die Lautstärke
an der Seite gibt es an der Unterseite noch zwei Potis mit denen die Pegel für
das Handfunkgerät eingestellt werden können. Sie sollten bei der ersten
Inbetriebnahme auf jeweils 1/4 im Uhrzeigersinn eingestellt und dann nach
Gehör justiert werden. Bei Verwendung eines anderen Funkgeräts oder großen
Änderungen der Tonhöhe kann eine Nachjustierung notwendig sein.

Danach kann im Prinzip schon losgemorst werden. Zur Einstellung gibt es die
Taste &lt;Cmd&gt;. Wenn sie gedrückt und gehalten wird, können dem Keyer
Kommandos in CW gegeben werden. Hier ist die Liste der Kommandos:

* D?             - query the tx delay
* Dnnn           - set tx delay in ms
* KMD            - disable keyer memory
* KME            - enable keyer memory
* M?1            - query cw memory 1
* M?2            - query cw memory 2
* M1...          - program cw memory 1
* M2...          - program cw memory 2
* O?             - query tx off delay in dits
* Onn            - set tx off delay in dits
* SK             - switch to straight key
* SWD            - disable paddle swap
* SWE            - enable paddle swap
* T?             - query the tone frequency
* TMD            - disable training mode (TX)
* TME            - enable training mode (no TX)
* Tnnn           - set a tone frequency
* V?             - query the software version number
* W?             - query the keyer speed
* Wnn            - set the keyer speed in wpm

Die Kommandos werden jeweils mit einem hohen Ton quittiert, wenn sie
erfolgreich ausgeführt wurden. Bei einem Fehler (oder einem unbekannten
Kommando) erfolgt eine Fehlerquittung (niedriger Ton).

Alle Einstellungen werden ausfallsicher gespeichert und stehen deshalb nach
dem nächsten Start genau so wieder zur Verfügung.


### Wie funktioniert das mit den Speichern?

Der Keyer verfügt über zwei Speicherplätze. Diese können über die Kommandos
"M1" und "M2" programmiert und mit den Tasten &lt;1&gt; und &lt;2&gt;
abgerufen werden. Während ein Speichertext ausgegeben wird kann mittels Druck
auf einen beliebigen Knopf oder Betätigung der Morsetaste die Ausgabe
abgebrochen werden.

Für die Programmierung wird &lt;Cmd&gt; gedrückt und "M1" oder "M2" mit dem
Paddle gegeben. Dann folgt der zu speichernde Text. Eine Pause von 2 Sekunden
oder mehr beendet die Eingabe mit einem Quittungston.

Um Speicherplatz zu sparen, werden alle Zeiten auf volle Vielfache der für ein
"Dit" benötigten Zeit gerundet. Der benötigte Speicherplatz pro Element mit
folgender Pause (also "Dit" oder "Dah") beträgt 1 Byte. Insgesamt stehen etwa
200 Bytes pro Speicherplatz zur Verfügung. Wird die maximale Länge eines
Speichers bei der Eingabe überschritten, erfolgt eine Fehlerquittung und der
Text wird nicht gespeichert. Die Programmierung kann auch durch Loslassen der
&lt;Cmd&gt; Taste vorzeitig abgebrochen werden (z.B. bei Fehltastung). Der
bereits gespeicherte Text bleibt dann erhalten.


### Wieso produziert der Keyer die Tonausgabe des Handfunkgeräts?

Das ist bei Handfunkgeräten mit Kenwood "Norm" für den Anschluß eines externen
Mikrofons leider zwingend. Bei dieser Beschaltung werden zwei Stecker
verwendet, einer für Mikrofon/PTT, der andere für den Lautsprecher. Der
Stecker für Mikrofon/PTT führt aber keine Masse! Will man also PTT gegen Masse
schalten, muss zwingend auch der Lautsprecheranschluß verwendet werden. Dadurch
wird aber der interne Lautsprecher des Geräts abgeschaltet. Folglich muss die
Ausgabe über das angeschlossene Gerät erfolgen.

Wird eine andere Anschlußbeschaltung verwendet (von mir nicht getestet), dann
kann - falls das gewünscht ist - der "Speaker" Eingang am DIN Stecker des
Keyers einfach auf Masse gelegt, und der interne Lautsprecher des Geräts
verwendet werden. Der Mithörton kommt dann aus dem Lautsprecher des Keyers,
das Empfangssignal aus dem Lautsprecher des Funkgeräts.


### Welche Funkgeräte wurden getestet?

Ich habe den Keyer mit meinem Wouxun KG-UV6D und mit zwei verschiedenen PMR
Funkgeräten getestet. Alle diese Geräte hatten die Kenwood Anschlußbelegung.
Andere Anschlüsse sollten funktionieren, wenn PTT gegen Masse geschaltet wird
und keine hohen Ströme fliessen. Wenn keine Ausgabe des Empfangssignals über
die Keyer Hardware erfolgen soll, dann ist der entsprechende Eingang des
Keyers am DIN Stecker auf Masse zu legen.


### Was ist mit Iambic A/B?

Daran rätsle ich auch herum. Der Keyer implementiert Iambic A mit optionalem
Speicher. Ich selber benutze eigentlich immer Iambic B an anderen Funkgeräten,
habe aber mit dem Keyer überhaupt kein Problem, obwohl die Software definitiv
Iambic A implementiert. Ich werde das Thema noch etwas erforschen, vermute
aber, dass der Unterschied zwischen Iambic A und B normalerweise nicht zum
Tragen kommt und dass der Speicher viel entscheidender ist.

Falls ich feststellen sollte, dass es hier doch große Unterschiede gibt, werde
ich die Software entsprechend erweitern. Die Kommandos "IA" und "IB" sind
bereits für eine evtl. erfolgende Änderung der Software vorgesehen und werden
derzeit immer mit "Erfolg" quittiert. Für Rückmeldungen zum Thema (bitte kein
theoretisieren, sondern tatsächliche Tests mit dem Keyer!) bin ich dankbar.


### Ich habe ein Problem, wieso kann ich kein Issue anlegen?

Das Repository auf Github dient nur der Veröffentlichung, nicht der
Entwicklung. Für die Entwicklung wird ein anderes Repository verwendet.
Deshalb können auf Github keine Issues angelegt werden. Bei Problemen bitte
ich darum, mich per Mail zu kontaktieren.


### Wo gibt's den Keyer zu kaufen?

Gar nicht (zumindest bei mir). Von mir gibt es

  - den Schaltplan,
  - einen PCB Entwurf,
  - die Software,
  - OpenSCAD Sourcen und STL Datei für das Gehäuse,
  - und eine minimale Doku

auf GitHub. Mit dabei ist eine Teileliste mit (größtenteils und ohne Garantie)
Reichelt Bestellnummern.

Wenn genug Interesse besteht, kann ich nochmals einen Satz Platinen fertigen
lassen und - solange übrig sind - welche abgeben. GGf. mit programmierter AVR
CPU. Ich vertreibe aber keine Fertiggeräte oder Bausätze. Falls das jemand
machen will habe ich aber nichts dagegen.

