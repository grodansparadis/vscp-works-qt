#!/bin/sh
#pandoc -s -o documentation.html $(ls [0-9][0-9]*.md) --css "https://latex.vercel.app/style.css"
cp -r ../docs/images .
pandoc -s -f markdown -t html5 -o "start.html" "../docs/start.md" --css "https://latex.vercel.app/style.css"
pandoc -s -f markdown -t html5 -o "introduction.html" "../docs/introduction.md" --css "https://latex.vercel.app/style.css"
pandoc -s -f markdown -t html5 -o "connection_tcpip.html" "../docs/connection_tcpip.md" --css "https://latex.vercel.app/style.css"
