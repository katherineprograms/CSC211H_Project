# SkinFirst: Cosmetic Ingredient Analyzer

## Inspiration
Millions of people apply cosmetic products every day without knowing what is actually in them. Ingredient lists are long, technical, and written with chemicals that most people cannot read. This problem hits hardest for lower income communities and people of color, groups that are often the target market for affordable products that may contain the most harmful chemicals, yet have the least access to safety information.
The California Department of Public Health maintains a database of over 114,000 cosmetic products containing flagged carcinogens and reproductive toxins. It is free source of information, and almost nobody knows about it. This project was built to change that.

## What It Does
SkinFirst is a C++ Qt desktop application that loads the entire California Safe Cosmetics Program database and gives users a personalized, visual ingredient safety tool. Users can:

- Search any ingredient, product, or brand name across 114,635 real government records
- Set a personalized skin profile using a dropdown and three sliders for acne, dryness, and hyperpigmentation concern levels
- Analyze 500 ingredients in real time using a weighted safety and effectiveness scoring formula
- View color coded results: green for safe, orange for moderate risk, red for high risk
- Open a bar chart showing the 10 most dangerous flagged ingredients for their skin profile
- Export scored results to a CSV file
- Read an explanation of exactly how every score was calculated


## Challenges We Ran Into

The California Safe Cosmetics Program CSV contains product names like "Lip Color, Matte" where commas appear inside the data itself. A standard CSV split broke thousands of rows by treating those commas as column separators. The solution was writing a custom splitCSVLine() function that tracks whether the parser is inside quotation marks before deciding if a comma is a separator or part of the data.
Midway through development a hardware issue required switching to a different computer. After reinstalling Qt and opening the project the app crashed immediately on launch because the hardcoded file path to the CSV file was specific to the original computer. Rather than manually correcting dozens of references across multiple files, a directory class was created so all paths live in one place. Therefore only one file needs updating when the app moves to a new machine.
Learning Qt from scratch was a significant challenge since its signal, slot system, object model, and widget architecture are different from standard C++. Stack Overflow, the Qt API documentation, and a lot of trial and error were very helpful. Especially when debugging linker errors caused by missing Qt includes.


## Proud Accomplishments

- Built a fully functional C++ Qt GUI that loads and queries 114,635 real government records in real time
- Implemented a custom CSV parser that correctly handles quoted fields containing commas (a real world data problem that breaks most naive parsing approaches)
- Designed a two stage scoring algorithm combining safety data from the CSCP database with personalized skin type effectiveness scoring using C++ inheritance, templates, and a weighted formula
- Created a color coded results table, interactive bar chart popup, and three auto generated formulation suggestion cards that update dynamically after every analysis
- Connected real government safety data to an accessible visual interface built for communities that are disproportionately affected by harmful cosmetic ingredients


## What I Learned

- How to build a complete C++ Qt GUI application from scratch using signals, slots, widgets, and Qt Charts
- How to parse and query a large real world CSV dataset with 114,635 rows and handle edge cases like quoted commas and malformed rows using exception handling
- How to design a class hierarchy using inheritance ~ base class Ingredient with subclasses ActiveIngredient and AllergenIngredient that override scoring behavior
- How to implement a generic Score<T> template class that works for any numeric type and provides color coding and safety labels used directly by the UI
- How to use CMake to configure a Qt project, link external modules like Qt Charts, and automate file copying so the database is always available at runtime


## Built With

- C++ 17
- Qt 6 — Widgets and Qt Charts
- CMake 3.16
- California Safe Cosmetics Program Database ~ data.ca.gov
- Qt Creator IDE
- Git and GitHub


## Data Source
California Department of Public Health ~ Chemicals in Cosmetics
https://data.ca.gov/dataset/chemicals-in-cosmetics

## GitHub
https://github.com/katherineprograms/cosmetic-ingredient-analyzer
