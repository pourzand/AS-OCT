# Run this bash file inside the following docker container
export conf_quick_start=quick_start_template.yml
python -c "from simplemind import tools; tools.quick_start('${conf_quick_start}')"