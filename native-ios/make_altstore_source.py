#!/usr/bin/env python3
"""Genere la source AltStore (format AltStore Classic) pour l'IPA non signee.

Usage : make_altstore_source.py <App.app> <fichier.ipa> <url_ipa> <url_icone> <sortie.json>
Les permissions (privacy) sont lues dans l'Info.plist de l'app : AltStore
refuse l'installation si la source ne correspond pas au fichier.
"""
import json, os, plistlib, sys
from datetime import datetime, timezone

app_dir, ipa, ipa_url, icon_url, out = sys.argv[1:6]
with open(os.path.join(app_dir, "Info.plist"), "rb") as f:
    info = plistlib.load(f)

privacy = {k: v for k, v in info.items() if k.startswith("NS") and k.endswith("UsageDescription")}

source = {
    "name": "Carnet de bord",
    "subtitle": "Fiches mission véhicules",
    "description": "Source AltStore de l'application Carnet de bord (PREJ Marseille).",
    "iconURL": icon_url,
    "tintColor": "#1B3A6B",
    "apps": [{
        "name": "Carnet de bord",
        "bundleIdentifier": info["CFBundleIdentifier"],
        "developerName": "RCZ / PREJ Marseille",
        "subtitle": "Fiches mission véhicules",
        "localizedDescription": "Saisie des fiches mission véhicules, génération du PDF et envoi par Messages.",
        "iconURL": icon_url,
        "tintColor": "#1B3A6B",
        "category": "utilities",
        "versions": [{
            "version": str(info.get("CFBundleShortVersionString", "1.0")),
            "buildVersion": str(info.get("CFBundleVersion", "1")),
            "date": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
            "localizedDescription": "Nouvelle version générée automatiquement par GitHub.",
            "downloadURL": ipa_url,
            "size": os.path.getsize(ipa),
            "minOSVersion": str(info.get("MinimumOSVersion", "15.0")),
        }],
        "appPermissions": {
            "entitlements": [],
            "privacy": privacy,
        },
    }],
    "news": [],
}
with open(out, "w", encoding="utf-8") as f:
    json.dump(source, f, ensure_ascii=False, indent=2)
print(json.dumps(source, ensure_ascii=False, indent=2))
