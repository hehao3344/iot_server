{application,emqx_passwd,
             [{description,"Password Hash Library for EMQ X Broker"},
              {vsn,"0.1"},
              {modules, ['emqx_passwd']},
              {registered,[]},
              {applications,[kernel,stdlib,ssl,pbkdf2,bcrypt]},
              {env,[]}]}.