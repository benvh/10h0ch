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
| normal      | ESC  |
| eraser      | e    |
| marker      | m    |
| pencil      | p    |
| exit        | q    |

#### pencil tool

| action            | key |
| --                | --  |
| decrease pen size | [   |
| increase pen size | ]   |
| change color      | c   |

#### eraser tool

| action               | key |
| --                   | --  |
| decrease eraser size | [   |
| increase eraser size | ]   |

#### marker tool

| action                | key |
| --                    | --  |
| decrease marker size  | [   |
| increase marker size  | ]   |
| decrease number       | ;   |
| increase number       | '   |
| reset number to 1     | .   |
| change marker alpha   | a   |
| change marker color   | c   |
| toggle auto-increment | i   |

