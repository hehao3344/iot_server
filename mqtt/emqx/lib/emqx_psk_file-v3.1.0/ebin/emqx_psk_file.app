{application,emqx_psk_file,
             [{description,"EMQX PSK Plugin from File"},
              {vsn, "v3.1.0"},
              {modules, ['emqx_psk_file','emqx_psk_file_app','emqx_psk_file_sup']},
              {registered,[emqx_psk_file_sup]},
              {applications,[kernel,stdlib]},
              {mod,{emqx_psk_file_app,[]}}]}.
