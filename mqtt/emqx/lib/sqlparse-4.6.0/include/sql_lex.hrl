%% -----------------------------------------------------------------------------
%%
%% sql_lex.hrl: SQL - lexer definition.
%%
%% Copyright (c) 2012-18 K2 Informatics GmbH.  All Rights Reserved.
%%
%% This file is provided to you under the Apache License,
%% Version 2.0 (the "License"); you may not use this file
%% except in compliance with the License.  You may obtain
%% a copy of the License at
%%
%%   http://www.apache.org/licenses/LICENSE-2.0
%%
%% Unless required by applicable law or agreed to in writing,
%% software distributed under the License is distributed on an
%% "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
%% KIND, either express or implied.  See the License for the
%% specific language governing permissions and limitations
%% under the License.
%%
%% -----------------------------------------------------------------------------

-ifndef(SQL_LEX_HRL).
-define(SQL_LEX_HRL, true).

-include("sqlparse.hrl").

%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% May not be used as identifier !!!
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

-define(KEYWORDS, [
    {"^(?i)(ALL)$",             'ALL'},
    {"^(?i)(ALTER)$",           'ALTER'},
    {"^(?i)(AND)$",             'AND'},
    {"^(?i)(ANY)$",             'ANY'},
    {"^(?i)(AS)$",              'AS'},
    {"^(?i)(ASC)$",             'ASC'},
    {"^(?i)(BETWEEN)$",         'BETWEEN'},
    {"^(?i)(BY)$",              'BY'},
    {"^(?i)(CASE)$",            'CASE'},
    {"^(?i)(CHAR)$",            'CHAR'},
    {"^(?i)(CHECK)$",           'CHECK'},
    {"^(?i)(CLUSTER)$",         'CLUSTER'},
    {"^(?i)(CONNECT)$",         'CONNECT'},
    {"^(?i)(CREATE)$",          'CREATE'},
    {"^(?i)(DATE)$",            'DATE'},
    {"^(?i)(DEFAULT)$",         'DEFAULT'},
    {"^(?i)(DELETE)$",          'DELETE'},
    {"^(?i)(DESC)$",            'DESC'},
    {"^(?i)(DISTINCT)$",        'DISTINCT'},
    {"^(?i)(DROP)$",            'DROP'},
    {"^(?i)(ELSE)$",            'ELSE'},
    {"^(?i)(EXISTS)$",          'EXISTS'},
    {"^(?i)(FLOAT)$",           'FLOAT'},
    {"^(?i)(FROM)$",            'FROM'},
    {"^(?i)(GRANT)$",           'GRANT'},
    {"^(?i)(GROUP)$",           'GROUP'},
    {"^(?i)(HAVING)$",          'HAVING'},
    {"^(?i)(IDENTIFIED)$",      'IDENTIFIED'},
    {"^(?i)(IN)$",              'IN'},
    {"^(?i)(INDEX)$",           'INDEX'},
    {"^(?i)(INSERT)$",          'INSERT'},
    {"^(?i)(INTERSECT)$",       'INTERSECT'},
    {"^(?i)(INTO)$",            'INTO'},
    {"^(?i)(IS)$",              'IS'},
    {"^(?i)(LIKE)$",            'LIKE'},
    {"^(?i)(LONG)$",            'LONG'},
    {"^(?i)(MINUS)$",           'MINUS'},
    {"^(?i)(NOCYCLE)$",         'NOCYCLE'},
    {"^(?i)(NOT)$",             'NOT'},
    {"^(?i)(NULL)$",            'NULLX'},
    {"^(?i)(NUMBER)$",          'NUMBER'},
    {"^(?i)(OF)$",              'OF'},
    {"^(?i)(ON)$",              'ON'},
    {"^(?i)(OPTION)$",          'OPTION'},
    {"^(?i)(OR)$",              'OR'},
    {"^(?i)(ORDER)$",           'ORDER'},
    {"^(?i)(PRIOR)$",           'PRIOR'},
    {"^(?i)(PUBLIC)$",          'PUBLIC'},
    {"^(?i)(RAW)$",             'RAW'},
    {"^(?i)(REVOKE)$",          'REVOKE'},
    {"^(?i)(SELECT)$",          'SELECT'},
    {"^(?i)(SET)$",             'SET'},
    {"^(?i)(SOME)$",            'SOME'},
    {"^(?i)(START)$",           'START'},
    {"^(?i)(SYNONYM)$",         'SYNONYM'},
    {"^(?i)(TABLE)$",           'TABLE'},
    {"^(?i)(TABLESPACE)$",      'TABLESPACE'},
    {"^(?i)(TABLES)$",          'TABLES'},
    {"^(?i)(TEMPORARY)$",       'TEMPORARY'},
    {"^(?i)(THEN)$",            'THEN'},
    {"^(?i)(THROUGH)$",         'THROUGH'},
    {"^(?i)(TO)$",              'TO'},
    {"^(?i)(TRIGGER)$",         'TRIGGER'},
    {"^(?i)(TRUNCATE)$",        'TRUNCATE'},
    {"^(?i)(UNION)$",           'UNION'},
    {"^(?i)(UNIQUE)$",          'UNIQUE'},
    {"^(?i)(UPDATE)$",          'UPDATE'},
    {"^(?i)(USER)$",            'USER'},
    {"^(?i)(VALUES)$",          'VALUES'},
    {"^(?i)(VARCHAR2)$",        'VARCHAR2'},
    {"^(?i)(VIEW)$",            'VIEW'},
    {"^(?i)(WHERE)$",           'WHERE'},
    {"^(?i)(WITH)$",            'WITH'}
]).

%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Includes all terminals.
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

-define(TOKENPATTERNS, [
    {"^(?i)((GO[\s\t]*TO))$",   'GOTO'},
    {"^(?i)(ABS)$",             'FUNS'},
    {"^(?i)(ACOS)$",            'FUNS'},
    {"^(?i)(ADMIN)$",           'ADMIN'},
    {"^(?i)(ALL)$",             'ALL'},
    {"^(?i)(ALTER)$",           'ALTER'},
    {"^(?i)(AND)$",             'AND'},
    {"^(?i)(ANY)$",             'ANY'},
    {"^(?i)(AS)$",              'AS'},
    {"^(?i)(ASC)$",             'ASC'},
    {"^(?i)(ASIN)$",            'FUNS'},
    {"^(?i)(ATAN)$",            'FUNS'},
    {"^(?i)(ATAN2)$",           'FUNS'},
    {"^(?i)(AUTHENTICATION)$",  'AUTHENTICATION'},
    {"^(?i)(AUTHORIZATION)$",   'AUTHORIZATION'},
    {"^(?i)(AVG)$",             'FUNS'},
    {"^(?i)(BAG)$",             'BAG'},
    {"^(?i)(BEGIN)$",           'BEGIN'},
    {"^(?i)(BETWEEN)$",         'BETWEEN'},
    {"^(?i)(BFILE)$",           'BFILE'},
    {"^(?i)(BINARY_DOUBLE)$",   'BINARY_DOUBLE'},
    {"^(?i)(BINARY_FLOAT)$",    'BINARY_FLOAT'},
    {"^(?i)(BITMAP)$",          'BITMAP'},
    {"^(?i)(BLOB)$",            'BLOB'},
    {"^(?i)(BODY)$",            'BODY'},
    {"^(?i)(BOOL_AND)$",        'UFUN'},
    {"^(?i)(BOOL_OR)$",         'UFUN'},
    {"^(?i)(BY)$",              'BY'},
    {"^(?i)(CALL)$",            'CALL'},
    {"^(?i)(CASCADE)$",         'CASCADE'},
    {"^(?i)(CASE)$",            'CASE'},
    {"^(?i)(CHAR)$",            'CHAR'},
    {"^(?i)(CHECK)$",           'CHECK'},
    {"^(?i)(CLOB)$",            'CLOB'},
    {"^(?i)(CLOSE)$",           'CLOSE'},
    {"^(?i)(CLUSTER)$",         'CLUSTER'},
    {"^(?i)(COMMIT)$",          'COMMIT'},
    {"^(?i)(CONNECT)$",         'CONNECT'},
    {"^(?i)(CONSTRAINT)$",      'CONSTRAINT'},
    {"^(?i)(CONSTRAINTS)$",     'CONSTRAINTS'},
    {"^(?i)(CONTENTS)$",        'CONTENTS'},
    {"^(?i)(CONTEXT)$",         'CONTEXT'},
    {"^(?i)(CONTINUE)$",        'CONTINUE'},
    {"^(?i)(CORR)$",            'FUNS'},
    {"^(?i)(COS)$",             'FUNS'},
    {"^(?i)(COSH)$",            'FUNS'},
    {"^(?i)(COT)$",             'FUNS'},
    {"^(?i)(COUNT)$",           'FUNS'},
    {"^(?i)(COVAR_POP)$",       'FUNS'},
    {"^(?i)(COVAR_SAMP)$",      'FUNS'},
    {"^(?i)(CREATE)$",          'CREATE'},
    {"^(?i)(CROSS)$",           'CROSS'},
    {"^(?i)(CURRENT)$",         'CURRENT'},
    {"^(?i)(CURSOR)$",          'CURSOR'},
    {"^(?i)(DATABASE)$",        'DATABASE'},
    {"^(?i)(DATAFILES)$",       'DATAFILES'},
    {"^(?i)(DATE)$",            'DATE'},
    {"^(?i)(DECODE)$",          'FUNS'},
    {"^(?i)(DEFAULT)$",         'DEFAULT'},
    {"^(?i)(DEFERRED)$",        'DEFERRED'},
    {"^(?i)(DELEGATE)$",        'DELEGATE'},
    {"^(?i)(DELETE)$",          'DELETE'},
    {"^(?i)(DESC)$",            'DESC'},
    {"^(?i)(DIRECTORY)$",       'DIRECTORY'},
    {"^(?i)(DISTINCT)$",        'DISTINCT'},
    {"^(?i)(DOUBLE)$",          'DOUBLE'},
    {"^(?i)(DROP)$",            'DROP'},
    {"^(?i)(ELSE)$",            'ELSE'},
    {"^(?i)(ELSIF)$",           'ELSIF'},
    {"^(?i)(END)$",             'END'},
    {"^(?i)(ENTERPRISE)$",      'ENTERPRISE'},
    {"^(?i)(ESCAPE)$",          'ESCAPE'},
    {"^(?i)(EXCEPT)$",          'EXCEPT'},
    {"^(?i)(EXECUTE)$",         'EXECUTE'},
    {"^(?i)(EXISTS)$",          'EXISTS'},
    {"^(?i)(EXTERNALLY)$",      'EXTERNALLY'},
    {"^(?i)(FETCH)$",           'FETCH'},
    {"^(?i)(FILTER_WITH)$",     'FILTER_WITH'},
    {"^(?i)(FLOAT)$",           'FLOAT'},
    {"^(?i)(FORCE)$",           'FORCE'},
    {"^(?i)(FOREIGN)$",         'FOREIGN'},
    {"^(?i)(FOUND)$",           'FOUND'},
    {"^(?i)(FROM)$",            'FROM'},
    {"^(?i)(FULL)$",            'FULL'},
    {"^(?i)(FUNCTION)$",        'FUNCTION'},
    {"^(?i)(GLOBALLY)$",        'GLOBALLY'},
    {"^(?i)(GRANT)$",           'GRANT'},
    {"^(?i)(GROUP)$",           'GROUP'},
    {"^(?i)(HASHMAP)$",         'HASHMAP'},
    {"^(?i)(HAVING)$",          'HAVING'},
    {"^(?i)(HIERARCHY)$",       'HIERARCHY'},
    {"^(?i)(IDENTIFIED)$",      'IDENTIFIED'},
    {"^(?i)(IF)$",              'IF'},
    {"^(?i)(IMMEDIATE)$",       'IMMEDIATE'},
    {"^(?i)(IN)$",              'IN'},
    {"^(?i)(INCLUDING)$",       'INCLUDING'},
    {"^(?i)(INDEX)$",           'INDEX'},
    {"^(?i)(INDICATOR)$",       'INDICATOR'},
    {"^(?i)(INNER)$",           'INNER'},
    {"^(?i)(INSERT)$",          'INSERT'},
    {"^(?i)(INTERSECT)$",       'INTERSECT'},
    {"^(?i)(INTO)$",            'INTO'},
    {"^(?i)(INVALIDATION)$",    'INVALIDATION'},
    {"^(?i)(IS)$",              'IS'},
    {"^(?i)(JOIN)$",            'JOIN'},
    {"^(?i)(KEEP)$",            'KEEP'},
    {"^(?i)(KEY)$",             'KEY'},
    {"^(?i)(KEYLIST)$",         'KEYLIST'},
    {"^(?i)(LANGUAGE)$",        'LANGUAGE'},
    {"^(?i)(LEFT)$",            'LEFT'},
    {"^(?i)(LIKE)$",            'LIKE'},
    {"^(?i)(LINK)$",            'LINK'},
    {"^(?i)(LOCAL)$",           'LOCAL'},
    {"^(?i)(LOG)$",             'LOG'},
    {"^(?i)(LONG)$",            'LONG'},
    {"^(?i)(LOWER)$",           'FUNS'},
    {"^(?i)(LTRIM)$",           'FUNS'},
    {"^(?i)(MATERIALIZED)$",    'MATERIALIZED'},
    {"^(?i)(MAX)$",             'FUNS'},
    {"^(?i)(MEDIAN)$",          'FUNS'},
    {"^(?i)(MIN)$",             'FUNS'},
    {"^(?i)(MINUS)$",           'MINUS'},
    {"^(?i)(NATURAL)$",         'NATURAL'},
    {"^(?i)(NCHAR)$",           'NCHAR'},
    {"^(?i)(NCLOB)$",           'NCLOB'},
    {"^(?i)(NO)$",              'NO'},
    {"^(?i)(NOCYCLE)$",         'NOCYCLE'},
    {"^(?i)(NONE)$",            'NONE'},
    {"^(?i)(NORM_WITH)$",       'NORM_WITH'},
    {"^(?i)(NOT)$",             'NOT'},
    {"^(?i)(NULL)$",            'NULLX'},
    {"^(?i)(NUMBER)$",          'NUMBER'},
    {"^(?i)(NVARCHAR2)$",       'NVARCHAR2'},
    {"^(?i)(NVL)$",             'FUNS'},
    {"^(?i)(OF)$",              'OF'},
    {"^(?i)(ON)$",              'ON'},
    {"^(?i)(ONLINE)$",          'ONLINE'},
    {"^(?i)(OPEN)$",            'OPEN'},
    {"^(?i)(OPTION)$",          'OPTION'},
    {"^(?i)(OR)$",              'OR'},
    {"^(?i)(ORDER)$",           'ORDER'},
    {"^(?i)(ORDERED_SET)$",     'ORDERED_SET'},
    {"^(?i)(OUTER)$",           'OUTER'},
    {"^(?i)(PACKAGE)$",         'PACKAGE'},
    {"^(?i)(PARTITION)$",       'PARTITION'},
    {"^(?i)(PRECISION)$",       'PRECISION'},
    {"^(?i)(PRESERVE)$",        'PRESERVE'},
    {"^(?i)(PRIMARY)$",         'PRIMARY'},
    {"^(?i)(PRIOR)$",           'PRIOR'},
    {"^(?i)(PRIVILEGES)$",      'PRIVILEGES'},
    {"^(?i)(PROCEDURE)$",       'PROCEDURE'},
    {"^(?i)(PROFILE)$",         'PROFILE'},
    {"^(?i)(PUBLIC)$",          'PUBLIC'},
    {"^(?i)(PURGE)$",           'PURGE'},
    {"^(?i)(QUOTA)$",           'QUOTA'},
    {"^(?i)(RAW)$",             'RAW'},
    {"^(?i)(REAL)$",            'REAL'},
    {"^(?i)(REFERENCES)$",      'REFERENCES'},
    {"^(?i)(REGR_AVGX)$",       'FUNS'},
    {"^(?i)(REGR_AVGY)$",       'FUNS'},
    {"^(?i)(REGR_COUNT)$",      'FUNS'},
    {"^(?i)(REGR_INTERCEPT)$",  'FUNS'},
    {"^(?i)(REGR_R2)$",         'FUNS'},
    {"^(?i)(REGR_SLOPE)$",      'FUNS'},
    {"^(?i)(REGR_SXX)$",        'FUNS'},
    {"^(?i)(REGR_SXY)$",        'FUNS'},
    {"^(?i)(REGR_SYY)$",        'FUNS'},
    {"^(?i)(REQUIRED)$",        'REQUIRED'},
    {"^(?i)(RETURN)$",          'RETURN'},
    {"^(?i)(RETURNING)$",       'RETURNING'},
    {"^(?i)(REUSE)$",           'REUSE'},
    {"^(?i)(REVOKE)$",          'REVOKE'},
    {"^(?i)(RIGHT)$",           'RIGHT'},
    {"^(?i)(ROLE)$",            'ROLE'},
    {"^(?i)(ROLES)$",           'ROLES'},
    {"^(?i)(ROLLBACK)$",        'ROLLBACK'},
    {"^(?i)(ROWID)$",           'ROWID'},
    {"^(?i)(SCHEMA)$",          'SCHEMA'},
    {"^(?i)(SELECT)$",          'SELECT'},
    {"^(?i)(SELECTIVITY)$",     'UFUN'},
    {"^(?i)(SEQUENCE)$",        'SEQUENCE'},
    {"^(?i)(SET)$",             'SET'},
    {"^(?i)(SIN)$",             'FUNS'},
    {"^(?i)(SINH)$",            'FUNS'},
    {"^(?i)(SOME)$",            'SOME'},
    {"^(?i)(SQLERROR)$",        'SQLERROR'},
    {"^(?i)(START)$",           'START'},
    {"^(?i)(STDDEV)$",          'FUNS'},
    {"^(?i)(STDDEV_POP)$",      'FUNS'},
    {"^(?i)(STDDEV_SAMP)$",     'FUNS'},
    {"^(?i)(STORAGE)$",         'STORAGE'},
    {"^(?i)(SUM)$",             'FUNS'},
    {"^(?i)(SYNONYM)$",         'SYNONYM'},
    {"^(?i)(TABLE)$",           'TABLE'},
    {"^(?i)(TABLES)$",          'TABLES'},
    {"^(?i)(TABLESPACE)$",      'TABLESPACE'},
    {"^(?i)(TAN)$",             'FUNS'},
    {"^(?i)(TANH)$",            'FUNS'},
    {"^(?i)(TEMPORARY)$",       'TEMPORARY'},
    {"^(?i)(THEN)$",            'THEN'},
    {"^(?i)(THROUGH)$",         'THROUGH'},
    {"^(?i)(TIMESTAMP)$",       'TIMESTAMP'},
    {"^(?i)(TO)$",              'TO'},
    {"^(?i)(TO_CHAR)$",         'FUNS'},
    {"^(?i)(TO_DATE)$",         'FUNS'},
    {"^(?i)(TRIGGER)$",         'TRIGGER'},
    {"^(?i)(TRUNC)$",           'FUNS'},
    {"^(?i)(TRUNCATE)$",        'TRUNCATE'},
    {"^(?i)(TYPE)$",            'TYPE'},
    {"^(?i)(UNION)$",           'UNION'},
    {"^(?i)(UNIQUE)$",          'UNIQUE'},
    {"^(?i)(UNLIMITED)$",       'UNLIMITED'},
    {"^(?i)(UPDATE)$",          'UPDATE'},
    {"^(?i)(UPPER)$",           'FUNS'},
    {"^(?i)(UROWID)$",          'UROWID'},
    {"^(?i)(USER)$",            'USER'},
    {"^(?i)(USERS)$",           'USERS'},
    {"^(?i)(USING)$",           'USING'},
    {"^(?i)(VALIDATE)$",        'VALIDATE'},
    {"^(?i)(VALUES)$",          'VALUES'},
    {"^(?i)(VAR_POP)$",         'FUNS'},
    {"^(?i)(VAR_SAMP)$",        'FUNS'},
    {"^(?i)(VARCHAR2)$",        'VARCHAR2'},
    {"^(?i)(VARIANCE)$",        'FUNS'},
    {"^(?i)(VIEW)$",            'VIEW'},
    {"^(?i)(WHEN)$",            'WHEN'},
    {"^(?i)(WHENEVER)$",        'WHENEVER'},
    {"^(?i)(WHERE)$",           'WHERE'},
    {"^(?i)(WITH)$",            'WITH'},
    {"^(?i)(WORK)$",            'WORK'},
    {"^(?i)(XMLTYPE)$",         'XMLTYPE'}
]).

-endif.