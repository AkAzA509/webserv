#!/bin/bash

# Lire les données d'entrée de la requête POST
read -r input

# Extraire la valeur de la couleur à partir des données d'entrée
color=$(echo "$input" | sed -n 's/.*color=\([^&]*\).*/\1/p')

# Décode l'encodage URL (remplace les + par des espaces et les %xx par leur caractère ASCII)
color=$(echo "$color" | sed 's/+/ /g;s/%/\\x/g')

# En-tête HTTP
echo "Content-Type: text/html; charset=utf-8"
echo -e "\r\n\r\n"
echo ""

# Générer la page HTML avec la couleur de fond sélectionnée
cat << EOF
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <title>WebServ</title>
  <style>
    body { font-family: sans-serif; max-width: 600px; margin: 40px auto; }
    section { border: 1px solid ccc; border-radius: 6px; padding: 15px; margin: 20px 0; }
    button { padding: 8px 16px; font-size: 16px; margin-top: 10px; }
    input[type="file"] { margin-top: 10px; }
    pre { background: $color; padding: 10px; white-space: pre-wrap; }
    label { font-weight: bold; display: block; margin-top: 10px; }
    input[type="text"] { width: 100%; padding: 6px; margin-top: 5px; }
  </style>
</head>
<body>
  <h1>Et voila couleur changer !!!!!</h1>
  
</body>
</html>"
EOF