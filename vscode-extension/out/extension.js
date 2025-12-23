"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    var desc = Object.getOwnPropertyDescriptor(m, k);
    if (!desc || ("get" in desc ? !m.__esModule : desc.writable || desc.configurable)) {
      desc = { enumerable: true, get: function() { return m[k]; } };
    }
    Object.defineProperty(o, k2, desc);
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || (function () {
    var ownKeys = function(o) {
        ownKeys = Object.getOwnPropertyNames || function (o) {
            var ar = [];
            for (var k in o) if (Object.prototype.hasOwnProperty.call(o, k)) ar[ar.length] = k;
            return ar;
        };
        return ownKeys(o);
    };
    return function (mod) {
        if (mod && mod.__esModule) return mod;
        var result = {};
        if (mod != null) for (var k = ownKeys(mod), i = 0; i < k.length; i++) if (k[i] !== "default") __createBinding(result, mod, k[i]);
        __setModuleDefault(result, mod);
        return result;
    };
})();
Object.defineProperty(exports, "__esModule", { value: true });
exports.activate = activate;
exports.deactivate = deactivate;
const vscode = __importStar(require("vscode"));
const path = __importStar(require("path"));
const fs = __importStar(require("fs"));
const os = __importStar(require("os"));
// All agam keywords and built-in functions for autocomplete
const TAMIL_KEYWORDS = [
    // Control flow
    { label: 'என்றால்', kind: vscode.CompletionItemKind.Keyword, detail: 'if statement', insertText: 'என்றால் ${1:நிபந்தனை}:\n    ${2}' },
    { label: 'இல்லையென்றால்', kind: vscode.CompletionItemKind.Keyword, detail: 'elif statement', insertText: 'இல்லையென்றால் ${1:நிபந்தனை}:\n    ${2}' },
    { label: 'இல்லை', kind: vscode.CompletionItemKind.Keyword, detail: 'else statement', insertText: 'இல்லை:\n    ${1}' },
    { label: 'வரை', kind: vscode.CompletionItemKind.Keyword, detail: 'while loop', insertText: 'வரை ${1:நிபந்தனை}:\n    ${2}' },
    { label: 'ஒவ்வொரு', kind: vscode.CompletionItemKind.Keyword, detail: 'for loop', insertText: 'ஒவ்வொரு ${1:உறுப்பு} உள்ள ${2:பட்டியல்}:\n    ${3}' },
    { label: 'உள்ள', kind: vscode.CompletionItemKind.Keyword, detail: 'in operator' },
    { label: 'நிறுத்து', kind: vscode.CompletionItemKind.Keyword, detail: 'break' },
    { label: 'தொடர்', kind: vscode.CompletionItemKind.Keyword, detail: 'continue' },
    { label: 'திரும்பு', kind: vscode.CompletionItemKind.Keyword, detail: 'return', insertText: 'திரும்பு ${1}' },
    // Declarations
    { label: 'செயல்', kind: vscode.CompletionItemKind.Keyword, detail: 'function definition', insertText: 'செயல் ${1:பெயர்}(${2}):\n    ${3}' },
    { label: 'மாறி', kind: vscode.CompletionItemKind.Keyword, detail: 'variable declaration', insertText: 'மாறி ${1:பெயர்} = ${2}' },
    { label: 'மாறாத', kind: vscode.CompletionItemKind.Keyword, detail: 'constant declaration', insertText: 'மாறாத ${1:பெயர்} = ${2}' },
    { label: 'கட்டமைப்பு', kind: vscode.CompletionItemKind.Keyword, detail: 'struct definition', insertText: 'கட்டமைப்பு ${1:பெயர்}:\n    ${2:புலம்}' },
    { label: 'விருப்பம்', kind: vscode.CompletionItemKind.Keyword, detail: 'enum definition', insertText: 'விருப்பம் ${1:பெயர்}:\n    ${2:மதிப்பு}' },
    // Error handling
    { label: 'முயற்சி', kind: vscode.CompletionItemKind.Keyword, detail: 'try block', insertText: 'முயற்சி:\n    ${1}' },
    { label: 'பிடி', kind: vscode.CompletionItemKind.Keyword, detail: 'catch block', insertText: 'பிடி ${1:பிழை}:\n    ${2}' },
    { label: 'வீசு', kind: vscode.CompletionItemKind.Keyword, detail: 'throw error', insertText: 'வீசு(${1:"பிழை செய்தி"})' },
    { label: 'பொருத்து', kind: vscode.CompletionItemKind.Keyword, detail: 'pattern match', insertText: 'பொருத்து ${1}:\n    ${2} => ${3}' },
    // Boolean values
    { label: 'உண்மை', kind: vscode.CompletionItemKind.Value, detail: 'true' },
    { label: 'பொய்', kind: vscode.CompletionItemKind.Value, detail: 'false' },
    { label: 'இல்லா', kind: vscode.CompletionItemKind.Value, detail: 'null' },
    // Logical operators
    { label: 'மற்றும்', kind: vscode.CompletionItemKind.Operator, detail: 'and operator' },
    { label: 'அல்லது', kind: vscode.CompletionItemKind.Operator, detail: 'or operator' },
];
const BUILTIN_FUNCTIONS = [
    // I/O
    { label: 'அச்சிடு', kind: vscode.CompletionItemKind.Function, detail: 'Print to console', insertText: 'அச்சிடு(${1})' },
    { label: 'உள்ளீடு', kind: vscode.CompletionItemKind.Function, detail: 'Read user input', insertText: 'உள்ளீடு(${1:"உரை: "})' },
    // List/String functions
    { label: 'நீளம்', kind: vscode.CompletionItemKind.Function, detail: 'Get length', insertText: 'நீளம்(${1})' },
    { label: 'வகை', kind: vscode.CompletionItemKind.Function, detail: 'Get type', insertText: 'வகை(${1})' },
    { label: 'சேர்', kind: vscode.CompletionItemKind.Function, detail: 'Append to list', insertText: 'சேர்(${1:பட்டியல்}, ${2:உறுப்பு})' },
    { label: 'நீக்கு', kind: vscode.CompletionItemKind.Function, detail: 'Pop from list', insertText: 'நீக்கு(${1:பட்டியல்})' },
    { label: 'வரிசை', kind: vscode.CompletionItemKind.Function, detail: 'Sort list', insertText: 'வரிசை(${1:பட்டியல்})' },
    { label: 'தலைகீழ்', kind: vscode.CompletionItemKind.Function, detail: 'Reverse list', insertText: 'தலைகீழ்(${1:பட்டியல்})' },
    { label: 'வரம்பு', kind: vscode.CompletionItemKind.Function, detail: 'Generate range', insertText: 'வரம்பு(${1:முடிவு})' },
    // Math functions
    { label: 'வர்க்கம்', kind: vscode.CompletionItemKind.Function, detail: 'Square root', insertText: 'வர்க்கம்(${1})' },
    { label: 'அடி', kind: vscode.CompletionItemKind.Function, detail: 'Power', insertText: 'அடி(${1:அடிப்படை}, ${2:அடுக்கு})' },
    { label: 'தளம்', kind: vscode.CompletionItemKind.Function, detail: 'Floor', insertText: 'தளம்(${1})' },
    { label: 'கூரை', kind: vscode.CompletionItemKind.Function, detail: 'Ceiling', insertText: 'கூரை(${1})' },
    { label: 'முழுமை', kind: vscode.CompletionItemKind.Function, detail: 'Absolute value', insertText: 'முழுமை(${1})' },
    { label: 'குறைந்தபட்சம்', kind: vscode.CompletionItemKind.Function, detail: 'Minimum', insertText: 'குறைந்தபட்சம்(${1}, ${2})' },
    { label: 'அதிகபட்சம்', kind: vscode.CompletionItemKind.Function, detail: 'Maximum', insertText: 'அதிகபட்சம்(${1}, ${2})' },
    { label: 'தற்செயல்', kind: vscode.CompletionItemKind.Function, detail: 'Random number', insertText: 'தற்செயல்()' },
    { label: 'கூட்டு', kind: vscode.CompletionItemKind.Function, detail: 'Sum of list', insertText: 'கூட்டு(${1:பட்டியல்})' },
    // String functions
    { label: 'மேல்', kind: vscode.CompletionItemKind.Function, detail: 'Uppercase', insertText: 'மேல்(${1})' },
    { label: 'கீழ்', kind: vscode.CompletionItemKind.Function, detail: 'Lowercase', insertText: 'கீழ்(${1})' },
    { label: 'ஒழுங்கு', kind: vscode.CompletionItemKind.Function, detail: 'Trim whitespace', insertText: 'ஒழுங்கு(${1})' },
    { label: 'பிரி', kind: vscode.CompletionItemKind.Function, detail: 'Split string', insertText: 'பிரி(${1:சரம்}, ${2:பிரிப்பான்})' },
    { label: 'இணை', kind: vscode.CompletionItemKind.Function, detail: 'Join list', insertText: 'இணை(${1:பிரிப்பான்}, ${2:பட்டியல்})' },
    { label: 'மாற்று', kind: vscode.CompletionItemKind.Function, detail: 'Replace in string', insertText: 'மாற்று(${1:சரம்}, ${2:பழைய}, ${3:புதிய})' },
    { label: 'தொடங்கு', kind: vscode.CompletionItemKind.Function, detail: 'Starts with', insertText: 'தொடங்கு(${1:சரம்}, ${2:முன்னொட்டு})' },
    { label: 'முடிவு', kind: vscode.CompletionItemKind.Function, detail: 'Ends with', insertText: 'முடிவு(${1:சரம்}, ${2:பின்னொட்டு})' },
    { label: 'உள்ளதா', kind: vscode.CompletionItemKind.Function, detail: 'Contains', insertText: 'உள்ளதா(${1:சரம்}, ${2:துணைசரம்})' },
    // Type conversion
    { label: 'எண்ணாக', kind: vscode.CompletionItemKind.Function, detail: 'Convert to integer', insertText: 'எண்ணாக(${1})' },
    { label: 'தசமாக', kind: vscode.CompletionItemKind.Function, detail: 'Convert to float', insertText: 'தசமாக(${1})' },
    { label: 'சரமாக', kind: vscode.CompletionItemKind.Function, detail: 'Convert to string', insertText: 'சரமாக(${1})' },
    // File I/O
    { label: 'படி', kind: vscode.CompletionItemKind.Function, detail: 'Read file', insertText: 'படி(${1:"கோப்பு.txt"})' },
    { label: 'எழுது', kind: vscode.CompletionItemKind.Function, detail: 'Write file', insertText: 'எழுது(${1:"கோப்பு.txt"}, ${2:உள்ளடக்கம்})' },
    { label: 'உள்ளது', kind: vscode.CompletionItemKind.Function, detail: 'File exists', insertText: 'உள்ளது(${1:"கோப்பு.txt"})' },
    { label: 'வெளியேறு', kind: vscode.CompletionItemKind.Function, detail: 'Exit program', insertText: 'வெளியேறு(${1:0})' },
];
function activate(context) {
    console.log('agam extension activated!');
    // Register completion provider
    const completionProvider = vscode.languages.registerCompletionItemProvider('agam', {
        provideCompletionItems(document, position) {
            const completionItems = [];
            // Add keywords
            for (const kw of TAMIL_KEYWORDS) {
                const item = new vscode.CompletionItem(kw.label, kw.kind);
                item.detail = kw.detail;
                if (kw.insertText) {
                    item.insertText = new vscode.SnippetString(kw.insertText);
                }
                completionItems.push(item);
            }
            // Add built-in functions
            for (const fn of BUILTIN_FUNCTIONS) {
                const item = new vscode.CompletionItem(fn.label, fn.kind);
                item.detail = fn.detail;
                if (fn.insertText) {
                    item.insertText = new vscode.SnippetString(fn.insertText);
                }
                completionItems.push(item);
            }
            return completionItems;
        }
    }, '' // Trigger on any character
    );
    // Register run file command
    const runFileCommand = vscode.commands.registerCommand('agam.runFile', () => {
        const editor = vscode.window.activeTextEditor;
        if (!editor) {
            vscode.window.showErrorMessage('No file open');
            return;
        }
        const document = editor.document;
        if (document.languageId !== 'agam') {
            vscode.window.showErrorMessage('Not an agam file');
            return;
        }
        // Save the file first
        document.save().then(() => {
            const filePath = document.fileName;
            const config = vscode.workspace.getConfiguration('agam');
            const interpreterPath = config.get('interpreterPath', 'agam');
            // Create or show terminal
            let terminal = vscode.window.terminals.find((t) => t.name === 'agam');
            if (!terminal) {
                terminal = vscode.window.createTerminal('agam');
            }
            terminal.show();
            terminal.sendText(`${interpreterPath} "${filePath}"`);
        });
    });
    // Register run selection command
    const runSelectionCommand = vscode.commands.registerCommand('agam.runSelection', () => {
        const editor = vscode.window.activeTextEditor;
        if (!editor) {
            return;
        }
        const selection = editor.selection;
        const text = editor.document.getText(selection);
        if (!text) {
            vscode.window.showWarningMessage('No text selected');
            return;
        }
        // Create temp file and run
        const tempFile = path.join(os.tmpdir(), 'agam_temp.agam');
        fs.writeFileSync(tempFile, text);
        const config = vscode.workspace.getConfiguration('agam');
        const interpreterPath = config.get('interpreterPath', 'agam');
        let terminal = vscode.window.terminals.find((t) => t.name === 'agam');
        if (!terminal) {
            terminal = vscode.window.createTerminal('agam');
        }
        terminal.show();
        terminal.sendText(`${interpreterPath} "${tempFile}"`);
    });
    // Register open REPL command
    const openReplCommand = vscode.commands.registerCommand('agam.openRepl', () => {
        const config = vscode.workspace.getConfiguration('agam');
        const interpreterPath = config.get('interpreterPath', 'agam');
        let terminal = vscode.window.terminals.find((t) => t.name === 'agam REPL');
        if (!terminal) {
            terminal = vscode.window.createTerminal('agam REPL');
        }
        terminal.show();
        terminal.sendText(interpreterPath);
    });
    const hoverProvider = vscode.languages.registerHoverProvider('agam', {
        provideHover(document, position, _token) {
            const range = document.getWordRangeAtPosition(position, /[அ-ஔக-னa-zA-Z_][அ-ஔக-னa-zA-Z0-9_]*/);
            if (!range) {
                return;
            }
            const word = document.getText(range);
            // Find in keywords
            const keyword = TAMIL_KEYWORDS.find(k => k.label === word);
            if (keyword) {
                return new vscode.Hover(`**${keyword.label}** - ${keyword.detail}`);
            }
            // Find in built-ins
            const builtin = BUILTIN_FUNCTIONS.find(f => f.label === word);
            if (builtin) {
                return new vscode.Hover(`**${builtin.label}()** - ${builtin.detail}`);
            }
            return;
        }
    });
    context.subscriptions.push(completionProvider, runFileCommand, runSelectionCommand, openReplCommand, hoverProvider);
}
function deactivate() { }
//# sourceMappingURL=extension.js.map