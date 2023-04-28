Automatic QUality Assurance (AQUA)
===

## TL;DR: 
This tools can load a set of ROOT file, do user defined analysis and produce plots and `TTree`s for further plotting and those plots can be merged via [this tool](https://code.ihep.ac.cn/yanqy/merge_plot) to make parallel comparison.

## Configure file format
This is the compoments of a configuration file, for more details please check the example file in `example/` folder.
```json
{
  "files": [
    "/dybfs/users/chengjie/data/GENIE/GENIE3.0.6-wDeex/useroutput_wdeex.root",
    "/path/contains/many/root/files/*.root",
    "/path/contains/many/root/files/*/output.root"
  ],
  "treename": "atmgen", // tree name in the root file to send to analysis code
  "plot_file": "plot.root", // output file name to put the TH1 in 
  "preprocess": "CC_selection", // Optional, function name in the plugin, preprocess the input tree before sending to analysis code, this can include global cuts or global variable to be used
  "normalize": {  // Optional, if this block is given, the output TH1 will be normalized to the given value, using `Scale(norm, "WIDTH");`
    "func": "normalize_factor_CC", // func name given in the plugin file
    "parameters": [ // a list of parameters to be passed to the function
      "/media/storage/neutrino/GENIE/atmo/G18_10b_02_11b.root",
      "nu_mu_C12/tot_cc",
      "12"
    ]
  },  
  "plugins": [
    {
      "name": "Generator",  // name of the plugin
      "plugin": "libmuon_p.so", // plugin file path, use full path if nessary,
      "analysis": [
        {
          "treename": "treeout", // tree name to put the output tree in
          "filename": "muon_kin.root", // output file name to put the output tree in
          "func": "analysis", // function name in the plugin
          "output": [
            "muon_p",
            "muon_e",
            "leading_muon_e",
            "leading_muon_p" // variables to be captured in the output tree, can be variables found in input tree or variables calculated in the analysis function
          ],
          "plots": [
            {
              "var": "leading_muon_p",  // var to be plotted
              "name": "leading_muon_p", // name of the plot saved
              "xmin": 0,
              "xmax": 0,
              "bins": 128,
              "cut": "leading_muon_p>0.2" // cut can be formular to be evaled or func name of cut given in plugin file
            },
            {
              // can be more variables to plot
            }
          ],
          "stack_plots": [
            {
              "var": "dalphat",
              "name": "dalphat",
              "xmin": 0,
              "xmax": 0,
              "bins": 128,
              "cuts": [  //currently only support stacking different cuts on a same variable, can be more than 2 cuts
                "W < 1.4",
                "W > 1.4"
              ]
            }
          ]
        }
      ]
    }
  ]
}
```

## Analysis plugin
The analysis plugin is a shared library that can be build standalone or with the help of `cmake`. The plugin should contain several function with the following signature:
```C++
ROOT::RDF::RNode func_name(ROOT::RDF::RNode df);
```

For a normalize function, the signature should be:
```C++
double normalize_factor(ROOT::RDF::RNode df,
                           std::vector<std::string> parameters);
```

All cuts and analysis is done in the way of mutating the `RNode` and return it. You can find documentation of `RNode` [here](https://root.cern/doc/master/classROOT_1_1RDataFrame.html). Or read given code in `example/muon_p.cxx` and its comment.