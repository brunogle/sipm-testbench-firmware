#!/bin/bash

# === CONFIGURACIÓN ===
# Directorio que se va a respaldar
DIR_ORIGEN="/ruta/al/directorio"

# Carpeta temporal/local donde se guardará el archivo comprimido
DIR_BACKUP_LOCAL="/ruta/a/backups"

# Servidor remoto
SERVIDOR="usuario@mi.servidor.com"

# Carpeta en el servidor remoto donde se guardará el backup
DIR_REMOTO="/ruta/remota/backups"

# Prefijo del nombre del archivo
NOMBRE_ARCHIVO="backup_$(date +'%Y-%m-%d_%H-%M-%S').tar.gz"

# === COMPRESIÓN ===
mkdir -p "$DIR_BACKUP_LOCAL"
tar -czf "$DIR_BACKUP_LOCAL/$NOMBRE_ARCHIVO" -C "$DIR_ORIGEN" .

# === TRANSFERENCIA ===
scp "$DIR_BACKUP_LOCAL/$NOMBRE_ARCHIVO" "$SERVIDOR:$DIR_REMOTO"

# === LIMPIEZA OPCIONAL ===
# Elimina backups locales antiguos (por ejemplo, de más de 7 días)
find "$DIR_BACKUP_LOCAL" -type f -mtime +7 -delete