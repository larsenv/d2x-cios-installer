<table border='0'><tr><td width='32' height='20'><img src='http://img30.imageshack.us/img30/5585/44058607.gif' /></td><td>Official Homebrew to install d2x cIOS maintained by davebaol, xabby666 and XFlak</td></tr></table>
<br><b>FEATURES</b><br>
- d2x cIOS v4, v5 and v6 support<br>
- hybrid cIOS and potentially any cIOS support<br>
- module to add new cIOS in d2x cIOS installer without having to recompile it<br>
- available IOS bases: IOS37 v5662, IOS38 v4123, IOS53 v5662, IOS55 v5662, IOS56 v5661, IOS57 v5918, IOS58 v6175<br>
- installation slot support (200 --> 255 except slots 202, 222, 223, 224, 242 and 254)<br>
- online and offline installation (WAD support)<br>
- SD and USB devices support<br>
- classic controllers, Wiimotes and Game Cube pads support<br>
- argument --ios=XXX support in the meta.xml file to fix the IOS used by d2x cIOS installer (only active without AHBPROT)<br>
- argument --remove-cache support in the meta.xml file to delete the cache folder at the d2x cIOS installer exit<br>
- HW_AHBPROT support (enabled by default)<br>
- option to select the cIOS revision<br>
- batch installation support<br>
- module to save/load an installation config file<br>
- module to save a NUS script<br>
- uninstall cIOS feature restricted to the slots 200+<br>
<br><b>TODO</b><br>
- fix the issue related with cIOS without ES_Version patch<br>
- speed improvements<br>
- d2x cIOS installer documentation<br>
<br><b>KNOWN ISSUES</b><br>
- unable to downgrade the revision of an existing (c)IOS with a (c)IOS without the ES Version patch (Hermes's cIOS for example)<br>
- without AHBPROT and the --ios=XXX argument in the meta.xml file, an USB device will no longer be detected by d2x cIOS installer after an IOS reload<br>
<br>
<br>
<hr><br>
<br>
<table border='0'><tr><td width='32' height='20'><img src='http://img3.imageshack.us/img3/489/56862702.gif' /></td><td>Homebrew officiel pour installer les cIOS d2x de Davebaol, Xabby666 et XFlak</td></tr></table>
<br><b>FONCTIONNALITES</b><br>
- support des cIOS d2x v4, v5 et v6<br>
- support de n'importe quel cIOS y compris les cIOS "hybrides" composés de modules (ES, EHCI, DIPP...) bien spécifiques<br>
- IOS base disponibles: IOS37 v5662, IOS38 v4123, IOS53 v5662, IOS55 v5662, IOS56 v5661, IOS57 v5918, IOS58 v6175<br>
- possibilité de choisir le slot d'installation (200 à 255 à l'exception des slots 202, 222, 223, 224, 242 et 254)<br>
- support des installations Online et Offline (support fichiers WAD)<br>
- support des périphériques SD/USB<br>
- support des manettes classiques, des Wiimotes et des manettes Game Cube<br>
- prise en charge de l'argument --ios=XXX dans le meta.xml pour configurer automatiquement l'IOS à utiliser (uniquement actif en l'absence d'AHBPROT)<br>
- prise en charge de l'argument --remove-cache dans le meta.xml pour supprimer automatiquement le répertoire cache de d2x cIOS installer<br>
- support de HW_AHBPROT (activé par défaut)<br>
- possibilité d'ajouter des nouveaux cIOS dans d2x cIOS installer sans devoir le recompiler<br>
- possibilité de choisir la révision du cIOS parmi des valeurs pertinentes<br>
- possibilité d'installer plusieurs cIOS en une fois<br>
- possibilité d'enregistrer/charger la configuration d'une installation<br>
- possibilité de générer un script NUS adapté à la configuration de l'installation<br>
- possibilité de désinstaller un (c)IOS occupant un slot 200+<br>
<br><b>TODO</b><br>
- possibilité de downgrader la révision d'un (c)IOS existant avec un (c)IOS ne disposant pas du patch ES Version (cIOS d'Hermes par exemple)<br>
- documentation de d2x cIOS installer<br>
- certaines améliorations pour accélérer le module d'installation de d2x cIOS installer ???<br>
<br><b>BUGS CONNUS</b><br>
- impossible de downgrader la révision d'un (c)IOS existant avec un (c)IOS sans le patch ES Version (cIOS d'Hermes par exemple)<br>
- en l'absence d'AHBPROT et en n'utilisant pas l'argument --ios=XXX dans le meta.xml, d2x cIOS installer ne détectera plus un support USB après un IOS reload