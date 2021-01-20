# Unix man pages

Man pages are written in the markdown format and converted to correct format with pandoc.

Generate man-pages with

> pandoc -s -t man test.1.md -o test.1

Local man pages can be viewed with

> man ./my_man_page

Process is described [here](<https://unix.stackexchange.com/questions/6891/how-can-i-add-man-page-entries-for-my-own-power-tools>)


## Install manpages

> sudo cp examplecommand.1 /usr/local/share/man/man1/
> sudo mandb