# Symulator układów logicznych

Celem projektu jest zaprogramowanie symulatora układów logicznych, który mógłby służyć jako pomoc naukowa lub narzędzie do projektowania układów dla hobbystów. Dokładne wytyczne dotyczące projektu można znaleźć w [tym miejscu](https://science-cup.pl/wp-content/uploads/2021/11/MSC_2021_Symulator_ukladow_logicznych.pdf).

### Struktura projektu
```bash
.
├─ 📁 out (miejsce gdzie zostaną przygotowane pliki do kompilacji, etc)
├─ 📁 doc (dokumentacja)
├─ 📁 include (pliki z nagłówkami)
│  └─ 📄 [*.h]
├─ 📁 lib (zewnętrzne biblioteki)
│  ├─ 📄 CMakeLists.txt
│  └─ 📁 raylib (biblioteka graficzna)
├─ 📁 src (pliki z kodem źródłowym)
│  ├─ 📄 CMakeLists.txt
│  └─ 📄 [*.cpp]
├─ 📄 .gitignore
├─ 📄 CMakeLists.txt
└─ 📄 README.md
```
