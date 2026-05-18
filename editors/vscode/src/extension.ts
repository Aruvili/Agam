import * as path from 'path';
import * as fs from 'fs';
import { workspace, ExtensionContext } from 'vscode';

import {
	LanguageClient,
	LanguageClientOptions,
	ServerOptions,
	TransportKind
} from 'vscode-languageclient/node';

let client: LanguageClient;

export function activate(context: ExtensionContext) {
    let serverPath = workspace.getConfiguration('agam').get<string>('lsp.path');

    if (!serverPath || serverPath.trim() === '') {
        const repoBuildPath = path.join(context.extensionPath, '..', '..', 'build', 'bin', 'agam-lsp.exe');
        if (fs.existsSync(repoBuildPath)) {
            serverPath = repoBuildPath;
        } else {
            serverPath = "agam-lsp";
        }
    }

	const serverOptions: ServerOptions = {
		command: serverPath,
		transport: TransportKind.stdio
	};

	const clientOptions: LanguageClientOptions = {
		documentSelector: [{ scheme: 'file', language: 'agam' }]
	};

	client = new LanguageClient(
		'agamLanguageServer',
		'Agam Language Server',
		serverOptions,
		clientOptions
	);

	client.start();
}

export function deactivate(): Thenable<void> | undefined {
	if (!client) {
		return undefined;
	}
	return client.stop();
}
