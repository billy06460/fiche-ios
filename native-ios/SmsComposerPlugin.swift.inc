
// =====================================================================
//  Plugin maison "SmsComposer" (iPhone)
//  Ajoute a la fin d'AppDelegate.swift par le workflow GitHub.
//  Ouvre Messages avec destinataire, texte et PDF deja remplis.
//  Apple impose que l'utilisateur appuie lui-meme sur "Envoyer".
// =====================================================================

@objc(SmsComposerPlugin)
public class SmsComposerPlugin: CAPPlugin, CAPBridgedPlugin, MFMessageComposeViewControllerDelegate {
    public let identifier = "SmsComposerPlugin"
    public let jsName = "SmsComposer"
    public let pluginMethods: [CAPPluginMethod] = [
        CAPPluginMethod(name: "composeSms", returnType: CAPPluginReturnPromise),
        CAPPluginMethod(name: "canSend", returnType: CAPPluginReturnPromise)
    ]

    private var pendingCall: CAPPluginCall?

    @objc func canSend(_ call: CAPPluginCall) {
        call.resolve([
            "text": MFMessageComposeViewController.canSendText(),
            "attachments": MFMessageComposeViewController.canSendAttachments()
        ])
    }

    @objc func composeSms(_ call: CAPPluginCall) {
        DispatchQueue.main.async {
            guard MFMessageComposeViewController.canSendText() else {
                call.reject("Cet appareil ne peut pas envoyer de messages (Messages / iMessage non configuré).")
                return
            }
            guard self.pendingCall == nil else {
                call.reject("Un message est déjà ouvert.")
                return
            }

            let to = call.getString("to") ?? ""
            let recipients = to.split(separator: ",")
                .map { $0.trimmingCharacters(in: .whitespacesAndNewlines) }
                .filter { !$0.isEmpty }

            let vc = MFMessageComposeViewController()
            vc.messageComposeDelegate = self
            vc.recipients = recipients
            vc.body = call.getString("body") ?? ""

            if MFMessageComposeViewController.canSendAttachments() {
                let attachments = call.getArray("attachments", JSObject.self) ?? []
                for a in attachments {
                    guard let b64 = a["data"] as? String,
                          let data = Data(base64Encoded: b64, options: .ignoreUnknownCharacters) else { continue }
                    let name = (a["name"] as? String) ?? "fiche.pdf"
                    let lower = name.lowercased()
                    let type = lower.hasSuffix(".pdf") ? "com.adobe.pdf"
                        : (lower.hasSuffix(".png") ? "public.png" : "public.jpeg")
                    vc.addAttachmentData(data, typeIdentifier: type, filename: name)
                }
            }

            guard let presenter = self.bridge?.viewController else {
                call.reject("Affichage de Messages impossible.")
                return
            }
            self.pendingCall = call
            presenter.present(vc, animated: true)
        }
    }

    public func messageComposeViewController(_ controller: MFMessageComposeViewController,
                                             didFinishWith result: MessageComposeResult) {
        controller.dismiss(animated: true) {
            let status: String
            switch result {
            case .sent: status = "sent"
            case .cancelled: status = "cancelled"
            case .failed: status = "failed"
            @unknown default: status = "unknown"
            }
            self.pendingCall?.resolve(["status": status, "mode": "messages"])
            self.pendingCall = nil
        }
    }
}

// Ecran principal de l'app : enregistre le plugin maison au demarrage
class MainViewController: CAPBridgeViewController {
    override open func capacitorDidLoad() {
        bridge?.registerPluginInstance(SmsComposerPlugin())
    }
}
