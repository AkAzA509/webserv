#!/bin/bash

# Lire les données d'entrée de la requête POST
read -r input

# Extraire la valeur de la couleur à partir des données d'entrée
color=$(echo "$input" | sed -n 's/.*color=\([^&]*\).*/\1/p')

# Décode l'encodage URL (remplace les + par des espaces et les %xx par leur caractère ASCII)
color=$(echo "$color" | sed 's/+/ /g;s/%/\\x/g')

if [ "$color" = "rainbow" ]; then
  color="linear-gradient(90deg, red, orange, yellow, green, cyan, blue, violet)"
fi

# Générer la page HTML avec la couleur de fond sélectionnée
cat << EOF
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <title>WebServ</title>
  <style>
    html, body {
      height: 100%;
      margin: 0;
      padding: 0;
    }
    body {
      background: $color;
      font-family: 'Segoe UI', Arial, sans-serif;
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      flex-direction: column;
      margin: 0;
    }
    .container {
      background: rgba(255,255,255,0.85);
      border-radius: 12px;
      box-shadow: 0 2px 12px rgba(0,0,0,0.07);
      padding: 32px 40px;
      text-align: center;
      min-width: 320px;
    }
    h1 {
      margin: 0 0 10px 0;
      font-size: 2.1em;
      color: #222;
    }
    p {
      margin: 0;
      color: #444;
      font-size: 1.1em;
    }
</style>
</head>
<body>
</head>
<body>
  <div class="container">
    <h1>🎉 Couleur changée !</h1>
    <p>Le fond de la page a bien été mis à jour.</p>
    <button onclick="window.location.href='/'" style="margin-top:22px;padding:10px 24px;font-size:1em;border-radius:6px;border:none;background:#4a90e2;color:#fff;cursor:pointer;transition:background 0.2s;">Revenir à l'accueil</button>
  </div>
</body>
</body>
</html>
EOF