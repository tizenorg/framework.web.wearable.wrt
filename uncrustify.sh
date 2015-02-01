uncrustify -c uncrustify.cfg --no-backup `find . -regex "\(.*\.cpp\|.*\.h\|.*\.c\|.*\.cc\)" | grep -v "orm.h\|orm_generator.h"`
