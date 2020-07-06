#include "parse.h"

static Type *read_new_type_enum(Token **rest, Token *token);
static int expect_array_length(Token *token);

bool is_type_tokens(Token *token, AllowStaticOrNot ason, AllowExternOrNot aeon, AllowConstOrNot acon) {
  return (
       equal(token, "int")
    || equal(token, "char")
    || equal(token, "_Bool")
    || equal(token, "long")
    || equal(token, "void")
    || equal(token, "struct")
    || equal(token, "enum")
    || find_typedef(token->location, token->length)
    || ason == ALLOW_STATIC && equal(token, "static")
    || aeon == ALLOW_EXTERN && equal(token, "extern")
    || acon == ALLOW_CONST && equal(token, "const")
  );
}

// type    = "int"
//         | "char"
//         | "_Bool"
//         | "long"
//         | "void"
//         | "struct" identifer? ("{" members "}")?
//         | indentifer    (declared with typedef)
Type *read_type(Token **rest, Token *token, AllowStaticOrNot ason, AllowExternOrNot aeon, AllowConstOrNot acon) {
  Type *type;
  bool is_extern = false;
  if(aeon == ALLOW_EXTERN && equal(token, "extern")) {
    token = token->next;
    is_extern = true;
  }

  bool is_static = false;
  if (!is_extern && ason == ALLOW_STATIC && equal(token, "static")) {
    token = token->next;
    is_static = true;
  }

  bool is_const = false;
  if (!is_static && acon == ALLOW_CONST && equal(token, "const")) {
    token = token->next;
    is_const = true;
  }

  if(equal(token, "long")) {
    type = new_type_long();
    token = token->next;
  } else if(equal(token, "void")) {
    type = new_type_void();
    token = token->next;
  } else if(equal(token, "int")) {
    type = new_type_int();
    token = token->next;
  } else if(equal(token, "char")) {
    type = new_type_char();
    token = token->next;
  } else if(equal(token, "_Bool")) {
    type = new_type_bool();
    token = token->next;
  } else if(equal(token, "enum")) {
    type = read_new_type_enum(&token, token->next);
  } else if(equal(token, "struct")) {
    type = read_new_type_struct(&token, token->next);
  } else if (is_identifer_token(token)) {
    TypeDef *tdf = find_typedef(token->location, token->length);
    if (!tdf)
      return NULL;
    token = token->next;
    type = tdf->type;
  } else {
    return NULL;
  }

  while (equal(token, "*")) {
    type = new_type_pointer(type);
    token = token->next;
  }
  type->is_static = is_static;
  type->is_extern = is_extern;
  type->is_const = is_const;
  *rest = token;
  return type;
}

// reading enum type  = "enum" read_new_type_enum
// read_new_type_enum = identifer
//                    | identifer? "{" child ("," child )* ","? "}"
// child              = identifer ("=" number)?
static Type *read_new_type_enum(Token **rest, Token *token) {
  EnumTag *etag;
  // identifer
  char *tname;
  int tnamelen;
  if (is_identifer_token(token)) {
    tname = token->location;
    tnamelen = token->length;
    token = token->next;
  } else {
    tname = token->location;
    tnamelen = 0;
  }
  if (!equal(token, "{")) {
    if (!tnamelen)
      error_at(token, "expected { of enum tag");
    if (!find_enum_tag(tname, tnamelen))
      error_at(token, "used undefined identifer of enum tag");
    *rest = token;
    return new_type_enum();
  }
  if (find_tag_in_enum_tags(tname, tnamelen, now_scope->etags))
    error_at(token, "duplicated identifer of enum tag");
  new_enum_tag(tname, tnamelen);
  etag = now_scope->etags;

  // {
  token = token->next;

  int num = 0;
  while (!equal(token, "}")) {
    // identifer ("=" number)? ("," identifer ("="number)?)* ","?
    if (!is_identifer_token(token))
      error_at(token, "expected identifer of enum");
    if (find_in_enum_tag(token->location, token->length, etag))
      error_at(token, "duplicated identifer of enum");
    char *name = token->location;
    int namelen = token->length;
    token = token->next;
    if (equal(token, "=") && is_number_token(token->next)) {
      num = str_to_l(token->next->location, token->next->length);
      token = token->next->next;
    }
    if (find_in_vars(name, namelen, now_scope->vars)
      || find_in_typedefs(name, namelen, now_scope->tdfs)
      || find_in_enum_tags(name, namelen, now_scope->etags)
      || !(now_scope->parent) && find_function(name, namelen) )
      error_at(token, "duplicate scope declarations of variable/typedef/function/enum");
    new_enum(name, namelen, num++, etag);
    if (equal(token, "}"))
      break;
    if (!equal(token, ","))
      error_at(token, "expected ',' of enum");
    token = token->next;
  }

  // }
  token = token->next;

  *rest = token;
  return new_type_enum();
}

// ident? ( "{" member "}" )? ;
Type *read_new_type_struct(Token **rest, Token *token) {
  char *name = NULL;
  int namelen;
  if (is_identifer_token(token)) {
    name = token->location;
    namelen = token->length;
    token = token->next;
  }

  StructTag *stag = find_stag(name, namelen);
  if (!equal(token, "{")) {
    // calling already defined member and named struct tag
    if (stag) {
      *rest = token;
      return stag->type;
    }
    if (!name)
      error_at(token, "expected identifer of struct tag name");
    // calling undefined member struct tag (expected typedef statement)
    Type *type = new_type_struct(0, NULL);
    type->undefined_member = true;
    new_stag(name, namelen, type);

    *rest = token;
    return type;
  }
  token = token->next;

  Member head;
  Member *tail = &head;
  int offset = 0;
  while (!equal(token, "}")) {
    tail->next = read_member(&token, token, offset);
    tail = tail->next;
    offset += tail->type->size;
  }
  token = token->next;

  Type *type = new_type_struct(offset, head.next);

  if (name) {
    // named struct tag
    StructTag *stag = find_stag(name, namelen);
    if (stag && stag->type->undefined_member)
      // define member of struct tag of defined only tag name with typedef
      copy_type(stag->type, type);
    else
      // error handle of same name struct tag in same scope is in new_stag()
      new_stag(name, namelen, type);
  }

  *rest = token;
  return type;
}

Member *read_member(Token **rest, Token *token, int offset) {
  char *name;
  int namelen;
  Type *type = read_type(&token, token, DENY_STATIC, DENY_EXTERN, DENY_CONST);
  type = declarator(&token, token, type, &name, &namelen, DENY_OMIT_LENGTH);
  type = type_suffix(&token, token, type, DENY_OMIT_LENGTH);

  if (!equal(token, ";"))
    error_at(token, "expected ;");
  token = token->next;

  *rest = token;
  return new_member(name, namelen, type, offset);
}

// declarator = "*"* ("(" declarator ")" | identifer ) type_suffix
Type *type_ptr_suffix(Token **rest, Token *token, Type *type) {
  while (equal(token, "*")) {
    type = new_type_pointer(type);
    token = token->next;
  }
  *rest = token;
  return type;
}

Type *declarator(Token **rest, Token *token, Type *type, char **namep, int *namelenp, AllowOmitLengthOrNot aolon) {
  type = type_ptr_suffix(&token, token, type);

  if (equal(token, "(")) {
    Type *placeholder = calloc(1, sizeof(Type));
    Type *new_type = declarator(&token, token->next, placeholder, namep, namelenp, DENY_OMIT_LENGTH);
    if (!equal(token, ")"))
      error_at(token, "expected ) of nested type");
    *placeholder = *type_suffix(rest, token->next, type, DENY_OMIT_LENGTH);
    return new_type;
  }
  if (!is_identifer_token(token))
    error_at(token, "expected identifer of declaration with type");
  *namep = token->location;
  *namelenp = token->length;
  token = token->next;
  type = type_suffix(&token, token, type, aolon);
  *rest = token;
  return type;
}

// type_suffix       = ("[" num "]" type_suffix)?
Type *type_suffix(Token **rest, Token *token, Type *ancestor, AllowOmitLengthOrNot aolon) {
  if (!equal(token, "["))
    return ancestor;
  token = token->next;

  int length;
  if (is_number_token(token)) {
    length = str_to_l(token->location, token->length);
    token = token->next;
  } else {
    if (aolon != ALLOW_OMIT_LENGTH)
      error_at(token, "expected number token of array length");
    if (!equal(token, "]"))
      error_at(token, "expected ] of array difinition");
    length = expect_array_length(token);
  }
  if (!equal(token,"]"))
    error_at(token, "expected ]");
  token = token->next;

  Type *parent = type_suffix(&token, token, ancestor, DENY_OMIT_LENGTH);

  *rest = token;
  return new_type_array(parent, length);
}

static int expect_array_length(Token *token) {
  int length = 0;
  int dimension = 0;

  while (!equal(token, "="))
    token = token->next;
  token = token->next;

  if (is_string_token(token)) {
    String *string = new_string(token->location+1, token->length-2);
    return string->length; // includes tail's \0
  }

  while (token && !equal(token, ";")) {
    if (equal(token, "{")) {
      ++dimension;
      token = token->next;
      continue;
    }
    if (equal(token, "}")) {
      if (--dimension == 1)
        length++;
      token = token->next;
      continue;
    }
    if (equal(token, ",")) {
      token = token->next;
      continue;
    }

    for (int depth = 0; depth !=0 || token && (!equal(token, ",") && !equal(token, "}")); token = token->next) {
      if (equal(token, "(")) {
        depth++;
        continue;
      } else if (equal(token, ")")) {
        depth--;
        continue;
      }
    }
    if (dimension==1)
      length++;
  }

  if (length==0)
    error_at(token, "ommited array length is zero");
  return length;
}
