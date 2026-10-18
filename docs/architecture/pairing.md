# Pairing a phone

A household pairs a phone by scanning a QR code on the hub's screen. The phone then gets its own read-only broker login for that site. Pairing topics sit under `alphamon-pairing/<siteId>/`, apart from the site root, so phones never see them.

## 1. On the hub

- **Code:** 128-bit random value, base64url (22 characters), single use, expiring 10 minutes after it is shown or when the screen closes. One active code per site.
- **QR code:** `https://<app-host>/#pair=<siteId>.<code>`, where `app-host` comes from the Config File. With end-to-end encryption (**Proposed**) it becomes `#pair=<siteId>.<code>.<fp>`, where `fp` is base64url of the first 16 bytes of SHA-256 of the hub's raw 32-byte X25519 public key, so the phone can check the key the server relays.
- The raw code MUST NOT leave the hub except inside the QR code. The hub publishes only its hash.
- **Paired phones:** the hub lists each phone's name and paired date, with **Remove**. At most 10 phones.

## 2. Steps

1. The hub publishes `offer` = `<SHA-256 of the code, 64 hex>|<expiresIn s>`.
2. The phone opens the QR link and sends `POST /pair` with `{ "siteId", "code", "clientName" }` in the body (never in the URL).
3. The server checks the code against the offer in constant time, creates a read-only broker login for the phone and answers `201 { "phoneId", "username", "password", "brokerUrl", "site": { "id" } }`.
4. The server publishes `result` = `<phoneId>|<clientName>|<paired UTC>`; the hub adds the phone to its list.
5. Errors: `400 pairing_code_invalid` (unknown, reused, expired or cancelled: one error for all), `400 invalid_request`, `429 rate_limited` (5 attempts per minute per source address and per site).

## 3. Removing a phone

- **Remove** on the hub publishes `revoke` = `<phoneId>`. The server deletes that phone's broker login and drops its session at once.
- **Resync:** the hub's list is the source of truth. It publishes the retained `phones` list on every connect and after every pair or Remove. The server deletes any phone login of that site that isn't in the list, except phones paired less than 15 minutes ago (their `result` may still be on its way). `-` means the hub keeps no phones, so every phone of the site loses access, for example after a factory reset. An empty payload is ignored, because an empty retained publish deletes the retained message.

## 4. Signed-in members

A household member signed in to the app can watch the live view away from home without pairing. The server gives them a read-only member login for the site, with the same access as a paired phone. The hub's phone-list resync removes only phone logins, never member logins.
