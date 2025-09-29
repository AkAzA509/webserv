# 🧪 CONFIGURATIONS DE TEST WEBSERV

## 📋 Guide d'utilisation

```bash
# Utiliser une config de test
./bin/webserv test_configs/[nom_config].dlq
```

## 🔧 Configurations Disponibles

### 1. **basic.dlq** - Test de Base
- **Port** : 8080
- **Fonctionnalités** : GET, POST, autoindex
- **Usage** : Premier test de fonctionnement
- **Test** : `curl http://localhost:8080/`

### 2. **multi_server.dlq** - Multi-Serveurs
- **Ports** : 8080, 8081
- **Fonctionnalités** : Serveurs virtuels, upload
- **Usage** : Test multiplexage et server_name
- **Test** : 
  ```bash
  curl -H "Host: server1.test" http://localhost:8080/
  curl -H "Host: server2.test" http://localhost:8081/
  ```

### 3. **cgi_test.dlq** - CGI Intensif
- **Port** : 8080
- **Fonctionnalités** : Bash CGI, Python CGI, upload+CGI
- **Usage** : Test exécution CGI et gestion processus
- **Test** :
  ```bash
  curl -X POST -d "color=red" http://localhost:8080/cgi-bin/
  curl http://localhost:8080/python-cgi/
  ```

### 4. **upload_test.dlq** - Upload/Download
- **Port** : 8080
- **Fonctionnalités** : Upload gros fichiers, DELETE
- **Usage** : Test gestion fichiers volumineux
- **Test** :
  ```bash
  curl -X POST -F "file=@bigfile.txt" http://localhost:8080/upload/
  curl -X DELETE http://localhost:8080/delete-files/file.txt
  ```

### 5. **error_test.dlq** - Gestion Erreurs
- **Port** : 8080
- **Fonctionnalités** : Timeouts courts, body size limité
- **Usage** : Test codes d'erreur HTTP
- **Test** :
  ```bash
  curl -X POST http://localhost:8080/method-not-allowed/ # 405
  curl -X POST --data-binary @bigfile http://localhost:8080/tiny-body/ # 413
  ```

### 6. **redirect_test.dlq** - Redirections
- **Port** : 8080
- **Fonctionnalités** : return, redirections POST
- **Usage** : Test redirections et return
- **Test** :
  ```bash
  curl -v http://localhost:8080/redirect-me/ # 303
  curl -X POST -d "data" http://localhost:8080/post-and-redirect/ # 303
  ```

### 7. **stress_test.dlq** - Test de Charge
- **Ports** : 8080, 8081
- **Fonctionnalités** : Timeouts courts, connexions multiples
- **Usage** : Test robustesse sous charge
- **Test** :
  ```bash
  # Connexions simultanées
  for i in {1..50}; do curl http://localhost:8080/ & done
  ```

### 8. **evaluation.dlq** - Test d'Évaluation
- **Port** : 8080
- **Fonctionnalités** : Tous les aspects du sujet
- **Usage** : Configuration complète pour évaluation 42
- **Test** : Tous les tests du sujet

## 🎯 Commandes de Test Utiles

```bash
# Test de base
curl -v http://localhost:8080/

# Test POST avec fichier
curl -X POST -F "file=@test.txt" http://localhost:8080/upload/

# Test DELETE
curl -X DELETE http://localhost:8080/delete/file.txt

# Test CGI
curl -X POST -d "color=blue" http://localhost:8080/cgi-bin/change_color.sh

# Test autoindex
curl http://localhost:8080/put_file/

# Test gros upload
dd if=/dev/zero of=bigfile bs=1M count=10
curl -X POST -F "file=@bigfile" http://localhost:8080/upload/

# Test connexions simultanées
for i in {1..20}; do curl http://localhost:8080/ & done

# Test avec Host header
curl -H "Host: server1.test" http://localhost:8080/
```

## 🔍 Monitoring

```bash
# Voir les processus webserv
ps aux | grep webserv

# Voir les connexions réseau
netstat -tlnp | grep :808

# Surveiller les ressources
top -p $(pgrep webserv)
```