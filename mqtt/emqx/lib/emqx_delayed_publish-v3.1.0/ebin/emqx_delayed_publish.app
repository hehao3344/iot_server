{application,emqx_delayed_publish,
             [{description,"EMQ X Delayed Publish"},
              {vsn, "v3.1.0"},
              {modules, ['emqx_delayed_publish','emqx_delayed_publish_app','emqx_delayed_publish_sup']},
              {registered,[emqx_delayed_publish_sup]},
              {applications,[kernel,stdlib]},
              {mod,{emqx_delayed_publish_app,[]}}]}.
