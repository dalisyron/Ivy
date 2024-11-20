package imp.parser.antlr; // Generated from Imp.g4 by ANTLR 4.13.2
import org.antlr.v4.runtime.Lexer;
import org.antlr.v4.runtime.CharStream;
import org.antlr.v4.runtime.Token;
import org.antlr.v4.runtime.TokenStream;
import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.atn.*;
import org.antlr.v4.runtime.dfa.DFA;
import org.antlr.v4.runtime.misc.*;

@SuppressWarnings({"all", "warnings", "unchecked", "unused", "cast", "CheckReturnValue", "this-escape"})
public class ImpLexer extends Lexer {
	static { RuntimeMetaData.checkVersion("4.13.2", RuntimeMetaData.VERSION); }

	protected static final DFA[] _decisionToDFA;
	protected static final PredictionContextCache _sharedContextCache =
		new PredictionContextCache();
	public static final int
		T__0=1, T__1=2, TRUE=3, FALSE=4, IF=5, THEN=6, ELSE=7, DONE=8, WHILE=9, 
		DO=10, BEGIN=11, END=12, ASSIGN=13, PLUS=14, TIMES=15, EQUAL=16, LEQ=17, 
		LT=18, GEQ=19, GT=20, AND=21, OR=22, IMPLIES=23, PRECOND=24, POSTCOND=25, 
		INVARIANT=26, SEMICOLON=27, VARIABLE=28, INTEGER=29, WS=30;
	public static String[] channelNames = {
		"DEFAULT_TOKEN_CHANNEL", "HIDDEN"
	};

	public static String[] modeNames = {
		"DEFAULT_MODE"
	};

	private static String[] makeRuleNames() {
		return new String[] {
			"T__0", "T__1", "TRUE", "FALSE", "IF", "THEN", "ELSE", "DONE", "WHILE", 
			"DO", "BEGIN", "END", "ASSIGN", "PLUS", "TIMES", "EQUAL", "LEQ", "LT", 
			"GEQ", "GT", "AND", "OR", "IMPLIES", "PRECOND", "POSTCOND", "INVARIANT", 
			"SEMICOLON", "VARIABLE", "INTEGER", "WS"
		};
	}
	public static final String[] ruleNames = makeRuleNames();

	private static String[] makeLiteralNames() {
		return new String[] {
			null, "'('", "')'", "'true'", "'false'", "'if'", "'then'", "'else'", 
			"'done'", "'while'", "'do'", "'begin'", "'end'", "':='", "'+'", "'*'", 
			"'='", "'<='", "'<'", "'>='", "'>'", "'&&'", "'||'", "'==>'", "'preq'", 
			"'post'", "'invariant'", "';'"
		};
	}
	private static final String[] _LITERAL_NAMES = makeLiteralNames();
	private static String[] makeSymbolicNames() {
		return new String[] {
			null, null, null, "TRUE", "FALSE", "IF", "THEN", "ELSE", "DONE", "WHILE", 
			"DO", "BEGIN", "END", "ASSIGN", "PLUS", "TIMES", "EQUAL", "LEQ", "LT", 
			"GEQ", "GT", "AND", "OR", "IMPLIES", "PRECOND", "POSTCOND", "INVARIANT", 
			"SEMICOLON", "VARIABLE", "INTEGER", "WS"
		};
	}
	private static final String[] _SYMBOLIC_NAMES = makeSymbolicNames();
	public static final Vocabulary VOCABULARY = new VocabularyImpl(_LITERAL_NAMES, _SYMBOLIC_NAMES);

	/**
	 * @deprecated Use {@link #VOCABULARY} instead.
	 */
	@Deprecated
	public static final String[] tokenNames;
	static {
		tokenNames = new String[_SYMBOLIC_NAMES.length];
		for (int i = 0; i < tokenNames.length; i++) {
			tokenNames[i] = VOCABULARY.getLiteralName(i);
			if (tokenNames[i] == null) {
				tokenNames[i] = VOCABULARY.getSymbolicName(i);
			}

			if (tokenNames[i] == null) {
				tokenNames[i] = "<INVALID>";
			}
		}
	}

	@Override
	@Deprecated
	public String[] getTokenNames() {
		return tokenNames;
	}

	@Override

	public Vocabulary getVocabulary() {
		return VOCABULARY;
	}


	public ImpLexer(CharStream input) {
		super(input);
		_interp = new LexerATNSimulator(this,_ATN,_decisionToDFA,_sharedContextCache);
	}

	@Override
	public String getGrammarFileName() { return "Imp.g4"; }

	@Override
	public String[] getRuleNames() { return ruleNames; }

	@Override
	public String getSerializedATN() { return _serializedATN; }

	@Override
	public String[] getChannelNames() { return channelNames; }

	@Override
	public String[] getModeNames() { return modeNames; }

	@Override
	public ATN getATN() { return _ATN; }

	public static final String _serializedATN =
		"\u0004\u0000\u001e\u00b7\u0006\uffff\uffff\u0002\u0000\u0007\u0000\u0002"+
		"\u0001\u0007\u0001\u0002\u0002\u0007\u0002\u0002\u0003\u0007\u0003\u0002"+
		"\u0004\u0007\u0004\u0002\u0005\u0007\u0005\u0002\u0006\u0007\u0006\u0002"+
		"\u0007\u0007\u0007\u0002\b\u0007\b\u0002\t\u0007\t\u0002\n\u0007\n\u0002"+
		"\u000b\u0007\u000b\u0002\f\u0007\f\u0002\r\u0007\r\u0002\u000e\u0007\u000e"+
		"\u0002\u000f\u0007\u000f\u0002\u0010\u0007\u0010\u0002\u0011\u0007\u0011"+
		"\u0002\u0012\u0007\u0012\u0002\u0013\u0007\u0013\u0002\u0014\u0007\u0014"+
		"\u0002\u0015\u0007\u0015\u0002\u0016\u0007\u0016\u0002\u0017\u0007\u0017"+
		"\u0002\u0018\u0007\u0018\u0002\u0019\u0007\u0019\u0002\u001a\u0007\u001a"+
		"\u0002\u001b\u0007\u001b\u0002\u001c\u0007\u001c\u0002\u001d\u0007\u001d"+
		"\u0001\u0000\u0001\u0000\u0001\u0001\u0001\u0001\u0001\u0002\u0001\u0002"+
		"\u0001\u0002\u0001\u0002\u0001\u0002\u0001\u0003\u0001\u0003\u0001\u0003"+
		"\u0001\u0003\u0001\u0003\u0001\u0003\u0001\u0004\u0001\u0004\u0001\u0004"+
		"\u0001\u0005\u0001\u0005\u0001\u0005\u0001\u0005\u0001\u0005\u0001\u0006"+
		"\u0001\u0006\u0001\u0006\u0001\u0006\u0001\u0006\u0001\u0007\u0001\u0007"+
		"\u0001\u0007\u0001\u0007\u0001\u0007\u0001\b\u0001\b\u0001\b\u0001\b\u0001"+
		"\b\u0001\b\u0001\t\u0001\t\u0001\t\u0001\n\u0001\n\u0001\n\u0001\n\u0001"+
		"\n\u0001\n\u0001\u000b\u0001\u000b\u0001\u000b\u0001\u000b\u0001\f\u0001"+
		"\f\u0001\f\u0001\r\u0001\r\u0001\u000e\u0001\u000e\u0001\u000f\u0001\u000f"+
		"\u0001\u0010\u0001\u0010\u0001\u0010\u0001\u0011\u0001\u0011\u0001\u0012"+
		"\u0001\u0012\u0001\u0012\u0001\u0013\u0001\u0013\u0001\u0014\u0001\u0014"+
		"\u0001\u0014\u0001\u0015\u0001\u0015\u0001\u0015\u0001\u0016\u0001\u0016"+
		"\u0001\u0016\u0001\u0016\u0001\u0017\u0001\u0017\u0001\u0017\u0001\u0017"+
		"\u0001\u0017\u0001\u0018\u0001\u0018\u0001\u0018\u0001\u0018\u0001\u0018"+
		"\u0001\u0019\u0001\u0019\u0001\u0019\u0001\u0019\u0001\u0019\u0001\u0019"+
		"\u0001\u0019\u0001\u0019\u0001\u0019\u0001\u0019\u0001\u001a\u0001\u001a"+
		"\u0001\u001b\u0001\u001b\u0005\u001b\u00a7\b\u001b\n\u001b\f\u001b\u00aa"+
		"\t\u001b\u0001\u001c\u0004\u001c\u00ad\b\u001c\u000b\u001c\f\u001c\u00ae"+
		"\u0001\u001d\u0004\u001d\u00b2\b\u001d\u000b\u001d\f\u001d\u00b3\u0001"+
		"\u001d\u0001\u001d\u0000\u0000\u001e\u0001\u0001\u0003\u0002\u0005\u0003"+
		"\u0007\u0004\t\u0005\u000b\u0006\r\u0007\u000f\b\u0011\t\u0013\n\u0015"+
		"\u000b\u0017\f\u0019\r\u001b\u000e\u001d\u000f\u001f\u0010!\u0011#\u0012"+
		"%\u0013\'\u0014)\u0015+\u0016-\u0017/\u00181\u00193\u001a5\u001b7\u001c"+
		"9\u001d;\u001e\u0001\u0000\u0004\u0002\u0000AZaz\u0003\u000009AZaz\u0001"+
		"\u000009\u0003\u0000\t\n\r\r  \u00b9\u0000\u0001\u0001\u0000\u0000\u0000"+
		"\u0000\u0003\u0001\u0000\u0000\u0000\u0000\u0005\u0001\u0000\u0000\u0000"+
		"\u0000\u0007\u0001\u0000\u0000\u0000\u0000\t\u0001\u0000\u0000\u0000\u0000"+
		"\u000b\u0001\u0000\u0000\u0000\u0000\r\u0001\u0000\u0000\u0000\u0000\u000f"+
		"\u0001\u0000\u0000\u0000\u0000\u0011\u0001\u0000\u0000\u0000\u0000\u0013"+
		"\u0001\u0000\u0000\u0000\u0000\u0015\u0001\u0000\u0000\u0000\u0000\u0017"+
		"\u0001\u0000\u0000\u0000\u0000\u0019\u0001\u0000\u0000\u0000\u0000\u001b"+
		"\u0001\u0000\u0000\u0000\u0000\u001d\u0001\u0000\u0000\u0000\u0000\u001f"+
		"\u0001\u0000\u0000\u0000\u0000!\u0001\u0000\u0000\u0000\u0000#\u0001\u0000"+
		"\u0000\u0000\u0000%\u0001\u0000\u0000\u0000\u0000\'\u0001\u0000\u0000"+
		"\u0000\u0000)\u0001\u0000\u0000\u0000\u0000+\u0001\u0000\u0000\u0000\u0000"+
		"-\u0001\u0000\u0000\u0000\u0000/\u0001\u0000\u0000\u0000\u00001\u0001"+
		"\u0000\u0000\u0000\u00003\u0001\u0000\u0000\u0000\u00005\u0001\u0000\u0000"+
		"\u0000\u00007\u0001\u0000\u0000\u0000\u00009\u0001\u0000\u0000\u0000\u0000"+
		";\u0001\u0000\u0000\u0000\u0001=\u0001\u0000\u0000\u0000\u0003?\u0001"+
		"\u0000\u0000\u0000\u0005A\u0001\u0000\u0000\u0000\u0007F\u0001\u0000\u0000"+
		"\u0000\tL\u0001\u0000\u0000\u0000\u000bO\u0001\u0000\u0000\u0000\rT\u0001"+
		"\u0000\u0000\u0000\u000fY\u0001\u0000\u0000\u0000\u0011^\u0001\u0000\u0000"+
		"\u0000\u0013d\u0001\u0000\u0000\u0000\u0015g\u0001\u0000\u0000\u0000\u0017"+
		"m\u0001\u0000\u0000\u0000\u0019q\u0001\u0000\u0000\u0000\u001bt\u0001"+
		"\u0000\u0000\u0000\u001dv\u0001\u0000\u0000\u0000\u001fx\u0001\u0000\u0000"+
		"\u0000!z\u0001\u0000\u0000\u0000#}\u0001\u0000\u0000\u0000%\u007f\u0001"+
		"\u0000\u0000\u0000\'\u0082\u0001\u0000\u0000\u0000)\u0084\u0001\u0000"+
		"\u0000\u0000+\u0087\u0001\u0000\u0000\u0000-\u008a\u0001\u0000\u0000\u0000"+
		"/\u008e\u0001\u0000\u0000\u00001\u0093\u0001\u0000\u0000\u00003\u0098"+
		"\u0001\u0000\u0000\u00005\u00a2\u0001\u0000\u0000\u00007\u00a4\u0001\u0000"+
		"\u0000\u00009\u00ac\u0001\u0000\u0000\u0000;\u00b1\u0001\u0000\u0000\u0000"+
		"=>\u0005(\u0000\u0000>\u0002\u0001\u0000\u0000\u0000?@\u0005)\u0000\u0000"+
		"@\u0004\u0001\u0000\u0000\u0000AB\u0005t\u0000\u0000BC\u0005r\u0000\u0000"+
		"CD\u0005u\u0000\u0000DE\u0005e\u0000\u0000E\u0006\u0001\u0000\u0000\u0000"+
		"FG\u0005f\u0000\u0000GH\u0005a\u0000\u0000HI\u0005l\u0000\u0000IJ\u0005"+
		"s\u0000\u0000JK\u0005e\u0000\u0000K\b\u0001\u0000\u0000\u0000LM\u0005"+
		"i\u0000\u0000MN\u0005f\u0000\u0000N\n\u0001\u0000\u0000\u0000OP\u0005"+
		"t\u0000\u0000PQ\u0005h\u0000\u0000QR\u0005e\u0000\u0000RS\u0005n\u0000"+
		"\u0000S\f\u0001\u0000\u0000\u0000TU\u0005e\u0000\u0000UV\u0005l\u0000"+
		"\u0000VW\u0005s\u0000\u0000WX\u0005e\u0000\u0000X\u000e\u0001\u0000\u0000"+
		"\u0000YZ\u0005d\u0000\u0000Z[\u0005o\u0000\u0000[\\\u0005n\u0000\u0000"+
		"\\]\u0005e\u0000\u0000]\u0010\u0001\u0000\u0000\u0000^_\u0005w\u0000\u0000"+
		"_`\u0005h\u0000\u0000`a\u0005i\u0000\u0000ab\u0005l\u0000\u0000bc\u0005"+
		"e\u0000\u0000c\u0012\u0001\u0000\u0000\u0000de\u0005d\u0000\u0000ef\u0005"+
		"o\u0000\u0000f\u0014\u0001\u0000\u0000\u0000gh\u0005b\u0000\u0000hi\u0005"+
		"e\u0000\u0000ij\u0005g\u0000\u0000jk\u0005i\u0000\u0000kl\u0005n\u0000"+
		"\u0000l\u0016\u0001\u0000\u0000\u0000mn\u0005e\u0000\u0000no\u0005n\u0000"+
		"\u0000op\u0005d\u0000\u0000p\u0018\u0001\u0000\u0000\u0000qr\u0005:\u0000"+
		"\u0000rs\u0005=\u0000\u0000s\u001a\u0001\u0000\u0000\u0000tu\u0005+\u0000"+
		"\u0000u\u001c\u0001\u0000\u0000\u0000vw\u0005*\u0000\u0000w\u001e\u0001"+
		"\u0000\u0000\u0000xy\u0005=\u0000\u0000y \u0001\u0000\u0000\u0000z{\u0005"+
		"<\u0000\u0000{|\u0005=\u0000\u0000|\"\u0001\u0000\u0000\u0000}~\u0005"+
		"<\u0000\u0000~$\u0001\u0000\u0000\u0000\u007f\u0080\u0005>\u0000\u0000"+
		"\u0080\u0081\u0005=\u0000\u0000\u0081&\u0001\u0000\u0000\u0000\u0082\u0083"+
		"\u0005>\u0000\u0000\u0083(\u0001\u0000\u0000\u0000\u0084\u0085\u0005&"+
		"\u0000\u0000\u0085\u0086\u0005&\u0000\u0000\u0086*\u0001\u0000\u0000\u0000"+
		"\u0087\u0088\u0005|\u0000\u0000\u0088\u0089\u0005|\u0000\u0000\u0089,"+
		"\u0001\u0000\u0000\u0000\u008a\u008b\u0005=\u0000\u0000\u008b\u008c\u0005"+
		"=\u0000\u0000\u008c\u008d\u0005>\u0000\u0000\u008d.\u0001\u0000\u0000"+
		"\u0000\u008e\u008f\u0005p\u0000\u0000\u008f\u0090\u0005r\u0000\u0000\u0090"+
		"\u0091\u0005e\u0000\u0000\u0091\u0092\u0005q\u0000\u0000\u00920\u0001"+
		"\u0000\u0000\u0000\u0093\u0094\u0005p\u0000\u0000\u0094\u0095\u0005o\u0000"+
		"\u0000\u0095\u0096\u0005s\u0000\u0000\u0096\u0097\u0005t\u0000\u0000\u0097"+
		"2\u0001\u0000\u0000\u0000\u0098\u0099\u0005i\u0000\u0000\u0099\u009a\u0005"+
		"n\u0000\u0000\u009a\u009b\u0005v\u0000\u0000\u009b\u009c\u0005a\u0000"+
		"\u0000\u009c\u009d\u0005r\u0000\u0000\u009d\u009e\u0005i\u0000\u0000\u009e"+
		"\u009f\u0005a\u0000\u0000\u009f\u00a0\u0005n\u0000\u0000\u00a0\u00a1\u0005"+
		"t\u0000\u0000\u00a14\u0001\u0000\u0000\u0000\u00a2\u00a3\u0005;\u0000"+
		"\u0000\u00a36\u0001\u0000\u0000\u0000\u00a4\u00a8\u0007\u0000\u0000\u0000"+
		"\u00a5\u00a7\u0007\u0001\u0000\u0000\u00a6\u00a5\u0001\u0000\u0000\u0000"+
		"\u00a7\u00aa\u0001\u0000\u0000\u0000\u00a8\u00a6\u0001\u0000\u0000\u0000"+
		"\u00a8\u00a9\u0001\u0000\u0000\u0000\u00a98\u0001\u0000\u0000\u0000\u00aa"+
		"\u00a8\u0001\u0000\u0000\u0000\u00ab\u00ad\u0007\u0002\u0000\u0000\u00ac"+
		"\u00ab\u0001\u0000\u0000\u0000\u00ad\u00ae\u0001\u0000\u0000\u0000\u00ae"+
		"\u00ac\u0001\u0000\u0000\u0000\u00ae\u00af\u0001\u0000\u0000\u0000\u00af"+
		":\u0001\u0000\u0000\u0000\u00b0\u00b2\u0007\u0003\u0000\u0000\u00b1\u00b0"+
		"\u0001\u0000\u0000\u0000\u00b2\u00b3\u0001\u0000\u0000\u0000\u00b3\u00b1"+
		"\u0001\u0000\u0000\u0000\u00b3\u00b4\u0001\u0000\u0000\u0000\u00b4\u00b5"+
		"\u0001\u0000\u0000\u0000\u00b5\u00b6\u0006\u001d\u0000\u0000\u00b6<\u0001"+
		"\u0000\u0000\u0000\u0004\u0000\u00a8\u00ae\u00b3\u0001\u0006\u0000\u0000";
	public static final ATN _ATN =
		new ATNDeserializer().deserialize(_serializedATN.toCharArray());
	static {
		_decisionToDFA = new DFA[_ATN.getNumberOfDecisions()];
		for (int i = 0; i < _ATN.getNumberOfDecisions(); i++) {
			_decisionToDFA[i] = new DFA(_ATN.getDecisionState(i), i);
		}
	}
}