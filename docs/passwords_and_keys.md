# Passwords and keys

VSCP Works needs secrets for many connections: passwords, AES encryption keys, HMAC keys, the password of a TLS key file and, for Level II drivers, an embedded driver configuration that often holds credentials. This page describes how they are stored and how you handle them on each platform.

## What is stored where

| What | Where |
|------|-------|
| Connection settings (host, user name, options, ...) | The configuration file |
| Passwords, keys and embedded Level II configurations | The keyring of the operating system |
| The same, when no keyring is available | `vscpworks-secrets.json` next to the configuration file |

The secrets of a connection are stored as one keyring entry:

| Platform | Keyring | Entry |
|----------|---------|-------|
| Linux (GNOME, Xfce, Cinnamon, ...) | GNOME Keyring, or any Secret Service keyring such as KeePassXC | Label `VSCP Works`, attribute `user` = `vscpworks/connection/{uuid}` |
| Linux (KDE) | KWallet | Folder `VSCP Works`, entry `vscpworks/connection/{uuid}` |
| Windows | Windows Credential Manager | Generic credential `vscpworks/connection/{uuid}` |
| macOS | Keychain (login keychain) | Service `VSCP Works`, account `vscpworks/connection/{uuid}` |

`{uuid}` is the `uuid` of the connection in the configuration file.

The configuration file is `~/.config/VSCP/vscp-works-qt.json` on Linux and macOS. On Windows it is in your local application data folder; VSCP Works prints the path (`Config file: ...`) when it starts. The configuration file, its backup (`.bak`) and `vscpworks-secrets.json` can only be read by your user.

### Configurations from earlier versions

Earlier versions stored passwords and keys in plain text in the configuration file. The first time a new version starts it moves them to the keyring, rewrites the configuration file and its backup without them and makes both readable by you only.

Copies you have made of the configuration file yourself are not changed. Delete them, as they still hold the passwords in plain text.

### Export and import

Exported connections (copy to the clipboard or save to a file) never contain passwords or keys, so an exported file can be shared. When you import a connection the dialog opens with the password and key fields empty; enter them there.

## Linux

### GNOME, Xfce, Cinnamon, MATE and others

The secrets are stored in GNOME Keyring through the Secret Service. Other Secret Service keyrings, such as KeePassXC with its Secret Service integration enabled, work the same way.

This needs VSCP Works to be built with libsecret (`libsecret-1-dev` on Debian and Ubuntu). Without it the build prints a warning and VSCP Works can only use KWallet, or the fallback file.

* **See the entries:** open *Passwords and Keys* (Seahorse), select the *Login* keyring and look for *VSCP Works*. From a terminal:

  ```bash
  secret-tool search --all server "VSCP Works"
  ```

  Note that this prints the secrets.
* **Remove an entry:**

  ```bash
  secret-tool clear server "VSCP Works" user "vscpworks/connection/{uuid}"
  ```

* **Locked keyring:** the login keyring is unlocked when you log in with your password. If you use automatic login, the desktop asks for the keyring password the first time VSCP Works needs a secret.

### KDE Plasma

The secrets are stored in KWallet, in the folder *VSCP Works* of your default wallet (normally *kdewallet*).

* **See the entries:** open *KWalletManager*, open the wallet and the folder *VSCP Works*. From a terminal:

  ```bash
  kwallet-query -f "VSCP Works" -l kdewallet
  ```

* **Remove an entry:** in KWalletManager, right click it and select *Delete*.

### No keyring

On servers, over SSH without a desktop session, in containers and on minimal window managers there is often no keyring. VSCP Works then stores the secrets in `~/.config/VSCP/vscpworks-secrets.json`, readable by your user only (permissions 0600). The file holds them in plain text, so use disk or home folder encryption on such systems.

When a keyring becomes available later (for example after installing GNOME Keyring), the secrets are moved into it the next time VSCP Works saves its settings.

## Windows

The secrets are stored in the Windows Credential Manager. They are encrypted with your Windows login and, in a domain with roaming profiles, follow you to other computers.

* **See the entries:** *Control Panel* > *Credential Manager* > *Windows Credentials*, under *Generic Credentials*, entries starting with `vscpworks/connection/`. From a command prompt:

  ```bat
  cmdkey /list | findstr vscpworks
  ```

* **Remove an entry:**

  ```bat
  cmdkey /delete:vscpworks/connection/{uuid}
  ```

A Windows credential can hold at most 2560 bytes. A large embedded Level II driver configuration does not fit; the secrets of that connection are then stored in `vscpworks-secrets.json`, readable by your user only.

## macOS

The secrets are stored in your login keychain.

* **See the entries:** open *Keychain Access*, select the *login* keychain and search for *VSCP Works*. From a terminal:

  ```bash
  security find-generic-password -s "VSCP Works" -a "vscpworks/connection/{uuid}" -w
  ```

  `-w` prints the secret.
* **Remove an entry:**

  ```bash
  security delete-generic-password -s "VSCP Works" -a "vscpworks/connection/{uuid}"
  ```

* **Access prompt:** macOS may ask whether VSCP Works may use the keychain. Select *Always Allow*. After an update of VSCP Works macOS can ask again.

## Creating keys

Keys are given as hex strings. Use a random key of the exact length, and never the same key for AES and HMAC.

| Key | Bytes | Hex characters |
|-----|-------|----------------|
| AES-128 | 16 | 32 |
| AES-192 | 24 | 48 |
| AES-256 | 32 | 64 |
| HMAC-SHA-256 | 32 | 64 |

Linux and macOS:

```bash
openssl rand -hex 32    # AES-256 or HMAC key
openssl rand -hex 16    # AES-128 key
```

Windows (PowerShell):

```powershell
$b = New-Object byte[] 32
[Security.Cryptography.RandomNumberGenerator]::Create().GetBytes($b)
($b | ForEach-Object { $_.ToString('x2') }) -join ''
```

Both ends of a connection must use the same keys: for example the `binary` section of the configuration of vscpl2drv-tcpipsrv for a tcp/ip connection in binary mode, or the UDP and multicast nodes and their firmware. Transfer keys to the other end over a secure channel, never by mail or chat in plain text.

## Moving to another computer

Copying the configuration file moves the connections but not their secrets. Either enter the passwords and keys again on the new computer, or move them with the keyring (Windows roaming profiles, an iCloud keychain, or a KeePassXC database).

## Removing secrets

Removing a connection in VSCP Works also removes its secrets from the keyring and from the fallback file. Entries left behind, for example from a configuration file you deleted, can be removed with the tools described above for each platform.

## Security notes

* A keyring protects the secrets when the computer is off and from other users. Programs that run as your user while the keyring is unlocked can usually read them.
* The fallback file is only protected by its permissions. Use disk encryption where it is used.
* Passwords and keys are not written to the VSCP Works log.
