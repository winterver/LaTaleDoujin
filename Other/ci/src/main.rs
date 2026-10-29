struct Cursor<'a> {
    ptr: &'a [char],
}

impl<'a> Cursor<'a> {
    fn new(buf: &'a [char]) -> Self {
        Self { ptr: buf }
    }

    fn next(&mut self) -> Option<char> {
        let (ch, ptr) = self.ptr.split_first()?;
        self.ptr = ptr;
        Some(*ch)
    }

    fn peek(&self) -> Option<char> {
        let (ch, _) = self.ptr.split_first()?;
        Some(*ch)
    }

    fn prefix(&mut self, str: &str) -> bool {
        let vec = str.chars().collect::<Vec<char>>();
        if vec.len() > self.ptr.len() { return false; }
        let result = vec == self.ptr[0..str.len()];
        if result { self.ptr = &self.ptr[str.len()..]; }
        result
    }
}

struct Lexer<'a> {
    cursor: Cursor<'a>,
    lineno: i32,
}

#[derive(Debug)]
enum Token {
    Id, Num, Str,
    Void, Char, Short, Int, Long, Enum,
    If, Else, Do, While, For, Continue, Break, Return, Sizeof,
    Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt,
    Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec,
    AddA, SubA, MulA, DivA, ModA, AndA, OrA, XorA, ShlA, ShrA,
    //~![](){}.,:;
    Una, Not, Lb, Rb, Lp, Rp, Lc, Rc, Dot, Cma, Coln, Sem,
}

impl<'a> Lexer<'a> {
    fn new(buf: &'a [char]) -> Self {
        Self { cursor: Cursor::new(buf), lineno: 1 }
    }

    fn next(&mut self) -> Option<(Token, i32, &[char])> {
        let cur = &mut self.cursor;
        let no = &mut self.lineno;
        while let Some(c) = cur.peek() {
            if " \t\r\n".contains(c) { cur.next(); *no += (c == '\n') as i32; }
            else if cur.prefix("#") { while let Some(c) = cur.peek() && c != '\n' { cur.next(); } }
            else if cur.prefix("//") { while let Some(c) = cur.peek() && c != '\n' { cur.next(); } }
            else if cur.prefix("/*") { while let Some(c) = cur.peek() && !cur.prefix("*/") { cur.next(); *no += (c == '\n') as i32; } }
            else if cur.prefix("*/") { panic!("({}) headless multiline-comment close (\"*/\")", *no); }
            else if cur.prefix("<<=") { return Some((Token::ShlA, *no, &[])); }
            else if cur.prefix(">>=") { return Some((Token::ShrA, *no, &[])); }
            else if cur.prefix("<<") { return Some((Token::Shl, *no, &[])); }
            else if cur.prefix(">>") { return Some((Token::Shr, *no, &[])); }
            else if cur.prefix("+=") { return Some((Token::AddA, *no, &[])); }
            else if cur.prefix("-=") { return Some((Token::SubA, *no, &[])); }
            else if cur.prefix("*=") { return Some((Token::MulA, *no, &[])); }
            else if cur.prefix("/=") { return Some((Token::DivA, *no, &[])); }
            else if cur.prefix("%=") { return Some((Token::ModA, *no, &[])); }
            else if cur.prefix("&=") { return Some((Token::AndA, *no, &[])); }
            else if cur.prefix("|=") { return Some((Token::OrA, *no, &[])); }
            else if cur.prefix("^=") { return Some((Token::XorA, *no, &[])); }
            else if cur.prefix("<=") { return Some((Token::Le, *no, &[])); }
            else if cur.prefix(">=") { return Some((Token::Ge, *no, &[])); }
            else if cur.prefix("!=") { return Some((Token::Ne, *no, &[])); }
            else if cur.prefix("==") { return Some((Token::Eq, *no, &[])); }
            else if cur.prefix("&&") { return Some((Token::Lan, *no, &[])); }
            else if cur.prefix("||") { return Some((Token::Lor, *no, &[])); }
            else if cur.prefix("++") { return Some((Token::Inc, *no, &[])); }
            else if cur.prefix("--") { return Some((Token::Dec, *no, &[])); }
            else if cur.prefix("+") { return Some((Token::Add, *no, &[])); }
            else if cur.prefix("-") { return Some((Token::Sub, *no, &[])); }
            else if cur.prefix("*") { return Some((Token::Mul, *no, &[])); }
            else if cur.prefix("/") { return Some((Token::Div, *no, &[])); }
            else if cur.prefix("%") { return Some((Token::Mod, *no, &[])); }
            else if cur.prefix("&") { return Some((Token::And, *no, &[])); }
            else if cur.prefix("|") { return Some((Token::Or, *no, &[])); }
            else if cur.prefix("^") { return Some((Token::Xor, *no, &[])); }
            else if cur.prefix("<") { return Some((Token::Lt, *no, &[])); }
            else if cur.prefix(">") { return Some((Token::Gt, *no, &[])); }
            else if cur.prefix("?") { return Some((Token::Cond, *no, &[])); }
            else if cur.prefix("=") { return Some((Token::Assign, *no, &[])); }
            else if cur.prefix("~") { return Some((Token::Una, *no, &[])); }
            else if cur.prefix("!") { return Some((Token::Not, *no, &[])); }
            else if cur.prefix("[") { return Some((Token::Lb, *no, &[])); }
            else if cur.prefix("]") { return Some((Token::Rb, *no, &[])); }
            else if cur.prefix("(") { return Some((Token::Lp, *no, &[])); }
            else if cur.prefix(")") { return Some((Token::Rp, *no, &[])); }
            else if cur.prefix("{") { return Some((Token::Lc, *no, &[])); }
            else if cur.prefix("}") { return Some((Token::Rc, *no, &[])); }
            else if cur.prefix(".") { return Some((Token::Dot, *no, &[])); }
            else if cur.prefix(",") { return Some((Token::Cma, *no, &[])); }
            else if cur.prefix(":") { return Some((Token::Coln, *no, &[])); }
            else if cur.prefix(";") { return Some((Token::Sem, *no, &[])); }
            else if c == '_' || c.is_alphabetic() {
                let (p, mut s, mut i) = (cur.ptr, cur.ptr, 0);
                while let Some(c) = cur.peek() && (c == '_' || c.is_alphanumeric()) { cur.next(); i += 1; s = &p[0..i]; }
                if s == "void".chars().collect::<Vec<char>>() { return Some((Token::Void, *no, &[])); }
                else if s == "char".chars().collect::<Vec<char>>() { return Some((Token::Char, *no, &[])); }
                else if s == "short".chars().collect::<Vec<char>>() { return Some((Token::Short, *no, &[])); }
                else if s == "int".chars().collect::<Vec<char>>() { return Some((Token::Int, *no, &[])); }
                else if s == "long".chars().collect::<Vec<char>>() { return Some((Token::Long, *no, &[])); }
                else if s == "enum".chars().collect::<Vec<char>>() { return Some((Token::Enum, *no, &[])); }
                else if s == "if".chars().collect::<Vec<char>>() { return Some((Token::If, *no, &[])); }
                else if s == "else".chars().collect::<Vec<char>>() { return Some((Token::Else, *no, &[])); }
                else if s == "do".chars().collect::<Vec<char>>() { return Some((Token::Do, *no, &[])); }
                else if s == "while".chars().collect::<Vec<char>>() { return Some((Token::While, *no, &[])); }
                else if s == "for".chars().collect::<Vec<char>>() { return Some((Token::For, *no, &[])); }
                else if s == "continue".chars().collect::<Vec<char>>() { return Some((Token::Continue, *no, &[])); }
                else if s == "break".chars().collect::<Vec<char>>() { return Some((Token::Break, *no, &[])); }
                else if s == "return".chars().collect::<Vec<char>>() { return Some((Token::Return, *no, &[])); }
                else if s == "sizeof".chars().collect::<Vec<char>>() { return Some((Token::Sizeof, *no, &[])); }
                else { return Some((Token::Id, *no, s)); }
            }
            else if c.is_digit(10) {
                let (p, mut i) = (cur.ptr, 0);
                cur.next(); i += 1;
                if let Some(c) = cur.peek() && "xX".contains(c) {
                    cur.next(); i += 1;
                    while let Some(c) = cur.peek() && c.is_digit(16) { cur.next(); i += 1; }
                    if i == 2 { panic!("({}) bad literal", *no); }
                }
                else if let Some(c) = cur.peek() && "bB".contains(c) {
                    cur.next(); i += 1;
                    while let Some(c) = cur.peek() && c.is_digit(2) { cur.next(); i += 1; }
                    if i == 2 { panic!("({}) bad literal", *no); }
                }
                else {
                    while let Some(c) = cur.peek() && c.is_digit(8) { cur.next(); i += 1; }
                }
                if let Some(c) = cur.peek() && c.is_alphanumeric() { panic!("({}) bad literal", *no); }
                return Some((Token::Num, *no, &p[0..i]));
            }
            else { panic!("({}) unknown character: '{}'", *no, c); }
        }
        None
    }
}

fn main() {
    let src = "
        int fib(){}
        int main() {
            int i[] = { 0233, 0x233, 0b123 };
            return fib();
        }
    ".chars().collect::<Vec<char>>();
    let mut lex = Lexer::new(&src);
    while let Some(t) = lex.next() {
        println!("{:?}", t);
    }
}
