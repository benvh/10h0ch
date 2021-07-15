# 10h0ch

simplistic image annotation tool

10h0ch will read whatever image (jpeg, png or TIFF) from stdin and render it in its window. basic tools to modify the image
are provided, e.g. pencils/erarser, text, ... On exit 10h0ch will dump the edited version of the image to
stdout as a PNG file.

### basic usage

```
10h0ch <path/to/some/image.png > annotated-image.png
```

### keybinds

keybinds are hardcoded (for now).

note that "exit" only works in "normal mode".

| tool/mode   | key  |
| ----        | ---  |
| exit        | q    |
| normal      | ESC  |
| pencil      | p    |
| eraser      | e    |

#### pencil tool

| action            | key |
| --                | --  |
| change color      | c   |
| decrease pen size | [   |
| increase pen size | ]   |

#### eraser tool

| action               | key |
| --                   | --  |
| decrease eraser size | [   |
| increase eraser size | ]   |
