{application,emqx_lua_hook,
             [{description,"EMQ X Lua Hooks"},
              {vsn, "v3.1-rc.2"},
              {modules, ['emqx_lua_hook','emqx_lua_hook_app','emqx_lua_hook_cli','emqx_lua_hook_sup','emqx_lua_script']},
              {registered,[]},
              {applications,[kernel,stdlib]},
              {mod,{emqx_lua_hook_app,[]}},
              {env,[]},
              {licenses,["Apache-2.0"]},
              {links,["Github","https://github.com/emqx/emqx-lua-hook"]}]}.