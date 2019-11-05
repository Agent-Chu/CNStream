## README

CNStream html doc pages generated by Sphinx.

### Prepared environment

Using virtualenv to build python3 working environment

### virtualenv + dep

```bash
virtualenv -p /usr/bin/python3 --no-site-package sphinx_env
source sphinx_env/bin/activate
pip install -i https://pypi.tuna.tsinghua.edu.cn/simple \
   -r requirements.txt
doxygen docs/Doxyfile.in
```
