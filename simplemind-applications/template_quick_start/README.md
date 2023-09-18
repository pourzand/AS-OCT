# SimpleMind Application for Kidney Segmentation with KiTS19


## Quick-start Pack for the Kidney Segmentation
Quick-start pack for generating a SimpleMind (SM) application for the Kidney Segmentation from KiTS19.
- **Kidney Semantic Network (SN)** `kidney_sn/`: 
    - SimpleMind knowledge base for the kidney segmentation on 3D abdomen CT 
    - The `kidney_sn` has cnn node pre-trained on KiTS19.
- **Quick-start configurations** `configurations/`
- **Quick-start template** `quick_start_template.yml`
- **Bash file to run the quick-start** `run_quick_start.sh`

### Semantic Network `kidney_sn`
![](./readme_sn_summary_relationship_graph.png)


## How to Quick-start

1. Update `quick_start_template.yml`
2. run quick start tool
    ```
    chmod 700 run_quick_start.sh
    ./run_quick_start.sh
    ```

## Data
After running quick-start tool, you will have the application directory in `sm_apps/ct_kidney_v1`.

Please download the data from https://kits19.grand-challenge.org/data/
and put the `data` directory in here (`sm_apps/ct_kidney_v1`).
